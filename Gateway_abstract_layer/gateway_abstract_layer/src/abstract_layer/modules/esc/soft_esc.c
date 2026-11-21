#include "soft_esc.h"

#include <string.h>

/*
 * Register model after ETG.1000.4 and the Beckhoff ET1100 / Microchip LAN9252
 * ESC documentation (register numbers are identical in both).
 */

/* ---- Register map --------------------------------------------------------- */

#define REG_TYPE          (0x0000U)
#define REG_REVISION      (0x0001U)
#define REG_BUILD         (0x0002U)
#define REG_FMMU_NUM      (0x0004U)
#define REG_SM_NUM        (0x0005U)
#define REG_RAM_SIZE      (0x0006U)
#define REG_PORT_DESC     (0x0007U)
#define REG_FEATURES      (0x0008U)
#define REG_STATION_ADR   (0x0010U)
#define REG_DL_STATUS     (0x0110U)
#define REG_AL_CONTROL    (0x0120U)
#define REG_AL_STATUS     (0x0130U)
#define REG_PDI_CONTROL   (0x0140U)
#define REG_AL_EVENT_MASK (0x0204U)
#define REG_AL_EVENT      (0x0220U)
#define REG_WD_STATUS     (0x0440U)
#define REG_EEP_CONTROL   (0x0502U)
#define REG_EEP_ADDRESS   (0x0504U)
#define REG_EEP_DATA      (0x0508U)
#define REG_FMMU0         (0x0600U)
#define REG_SM0           (0x0800U)
#define REG_PRAM          (0x1000U)

#define FMMU_SIZE         (16U)
#define SM_SIZE           (8U)

/* SM register offsets */
#define SM_START          (0U)
#define SM_LENGTH         (2U)
#define SM_CONTROL        (4U)
#define SM_STATUS         (5U)
#define SM_ACTIVATE       (6U)
#define SM_PDI_CONTROL    (7U)

/* FMMU register offsets */
#define FMMU_LOG_START    (0U)
#define FMMU_LENGTH       (4U)
#define FMMU_LOG_STARTBIT (6U)
#define FMMU_LOG_STOPBIT  (7U)
#define FMMU_PHYS_START   (8U)
#define FMMU_PHYS_BIT     (10U)
#define FMMU_TYPE         (11U)
#define FMMU_ACTIVATE     (12U)

/* AL event request bits (0x0220) */
#define AL_EVENT_CONTROL  ((uint32_t)0x00000001U)
#define AL_EVENT_SM_ACT   ((uint32_t)0x00000010U)
#define AL_EVENT_SM(n)    ((uint32_t)0x00000100U << (n))

/* EEPROM control/status (0x0502) */
#define EEP_CMD_MASK      (0x0700U)
#define EEP_CMD_READ      (0x0100U)
#define EEP_READ_8_BYTES  (0x0040U)

/* DL status: PDI operational + PDI watchdog ok + link on port 0 + port 0
 * open with communication, ports 1..3 closed (this slave is the last one). */
#define DL_STATUS_VALUE   (0x0001U | 0x0002U | 0x0010U | 0x0200U | 0x0400U \
                           | 0x1000U | 0x4000U)

#define ESC_TYPE_VALUE    (0xE5U) /* "emulated"; not a Beckhoff/Microchip code */
#define PDI_TYPE_ONCHIP   (0x80U) /* PDI: on-chip bus */

/* EtherCAT commands (ETG.1000.4) */
enum
{
    CMD_NOP = 0, CMD_APRD, CMD_APWR, CMD_APRW, CMD_FPRD, CMD_FPWR, CMD_FPRW,
    CMD_BRD, CMD_BWR, CMD_BRW, CMD_LRD, CMD_LWR, CMD_LRW, CMD_ARMW, CMD_FRMW
};

#define ETH_HEADER_LEN    (14U)
#define ETH_TYPE_ECAT     (0x88A4U)
#define ECAT_HEADER_LEN   (2U)
#define DG_HEADER_LEN     (10U)
#define DG_WKC_LEN        (2U)
#define DG_LEN_MASK       (0x07FFU)
#define DG_MORE           (0x8000U)

/* ---- Little-endian helpers ------------------------------------------------- */

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16)
            | ((uint32_t)p[3] << 24);
}

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)(v >> 8);
}

static void wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8) & 0xFFU);
    p[2] = (uint8_t)((v >> 16) & 0xFFU);
    p[3] = (uint8_t)(v >> 24);
}

static bool overlaps(uint32_t a, uint32_t alen, uint32_t b, uint32_t blen)
{
    return (a < (b + blen)) && (b < (a + alen));
}

/* ---- AL events and SyncManagers ------------------------------------------- */

static void al_event_set(soft_esc_t *esc, uint32_t bits)
{
    wr32(&esc->mem[REG_AL_EVENT], rd32(&esc->mem[REG_AL_EVENT]) | bits);
}

static void al_event_clear(soft_esc_t *esc, uint32_t bits)
{
    wr32(&esc->mem[REG_AL_EVENT], rd32(&esc->mem[REG_AL_EVENT]) & ~bits);
}

typedef struct
{
    uint16_t start;
    uint16_t length;
    bool     mailbox;    /* else buffered */
    bool     ecatWrites; /* direction: master writes (outputs / mailbox out) */
} sm_cfg_t;

static uint8_t *sm_reg(soft_esc_t *esc, uint32_t n)
{
    return &esc->mem[REG_SM0 + (n * SM_SIZE)];
}

static void sm_read_cfg(soft_esc_t *esc, uint32_t n, sm_cfg_t *cfg)
{
    const uint8_t *r = sm_reg(esc, n);
    uint8_t control = r[SM_CONTROL];

    cfg->start = rd16(&r[SM_START]);
    cfg->length = rd16(&r[SM_LENGTH]);
    cfg->mailbox = (control & 0x03U) == 0x02U;
    cfg->ecatWrites = ((control >> 2) & 0x03U) == 0x01U;
}

/* Enabled by the master and not deactivated by the PDI. */
static bool sm_get(soft_esc_t *esc, uint32_t n, sm_cfg_t *cfg)
{
    const uint8_t *r = sm_reg(esc, n);

    sm_read_cfg(esc, n, cfg);
    return ((r[SM_ACTIVATE] & 0x01U) != 0U)
            && ((r[SM_PDI_CONTROL] & 0x01U) == 0U)
            && (cfg->length != 0U);
}

/* Enabled by the master but deactivated by the PDI (e.g. SM2 outside OP):
 * the master cannot access the buffer and gets no working counter. */
static bool sm_pdi_deactivated(soft_esc_t *esc, uint32_t n, sm_cfg_t *cfg)
{
    const uint8_t *r = sm_reg(esc, n);

    sm_read_cfg(esc, n, cfg);
    return ((r[SM_ACTIVATE] & 0x01U) != 0U)
            && ((r[SM_PDI_CONTROL] & 0x01U) != 0U)
            && (cfg->length != 0U);
}

static void sm_update_status(soft_esc_t *esc, uint32_t n)
{
    uint8_t *r = sm_reg(esc, n);

    r[SM_STATUS] = esc->smFull[n] ? 0x08U : 0x00U;
}

static void sm_reset(soft_esc_t *esc, uint32_t n)
{
    esc->smFull[n] = false;
    al_event_clear(esc, AL_EVENT_SM(n));
    sm_update_status(esc, n);
}

/*
 * SyncManager rules for an ECAT (master) access. Returns false if the access
 * is refused (no data transfer, no working counter):
 *   mailbox, master writes : refused while full; last byte -> full, PDI event
 *   mailbox, master reads  : refused while empty; last byte -> empty, PDI event
 *   buffered, master writes: last byte -> PDI event (new outputs)
 *   master writes into a buffer the master reads (inputs): refused
 */
static bool sm_ecat_access(soft_esc_t *esc, uint32_t addr, uint32_t len, bool write,
        bool commit)
{
    uint32_t n;

    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        sm_cfg_t sm;
        bool lastByte;

        if (sm_pdi_deactivated(esc, n, &sm) && overlaps(addr, len, sm.start, sm.length))
        {
            return false;
        }
        if (!sm_get(esc, n, &sm) || !overlaps(addr, len, sm.start, sm.length))
        {
            continue;
        }
        lastByte = overlaps(addr, len, (uint32_t)sm.start + sm.length - 1U, 1U);

        if (write && !sm.ecatWrites)
        {
            return false;
        }
        if (sm.mailbox)
        {
            if (write && esc->smFull[n])
            {
                return false;
            }
            if (!write && sm.ecatWrites)
            {
                continue; /* master reading back its own mailbox: plain read */
            }
            if (!write && !esc->smFull[n])
            {
                return false;
            }
        }

        if (commit && lastByte)
        {
            if (sm.mailbox)
            {
                esc->smFull[n] = write;
                sm_update_status(esc, n);
                al_event_set(esc, AL_EVENT_SM(n));
            }
            else if (write)
            {
                al_event_set(esc, AL_EVENT_SM(n));
            }
            else
            {
                /* buffered input read by the master: nothing to signal */
            }
        }
    }
    return true;
}

/* SyncManager rules for a PDI (slave stack) access; never refused. */
static void sm_pdi_access(soft_esc_t *esc, uint32_t addr, uint32_t len, bool write)
{
    uint32_t n;

    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        sm_cfg_t sm;

        if (!sm_get(esc, n, &sm) || !overlaps(addr, len, sm.start, sm.length))
        {
            continue;
        }
        /* the PDI is the reader of master-written buffers and the writer of
         * master-read buffers */
        if (write == sm.ecatWrites)
        {
            continue;
        }
        if (overlaps(addr, len, sm.start, 1U))
        {
            al_event_clear(esc, AL_EVENT_SM(n)); /* first byte accessed */
        }
        if (sm.mailbox && overlaps(addr, len, (uint32_t)sm.start + sm.length - 1U, 1U))
        {
            esc->smFull[n] = write; /* PDI read empties, PDI write fills */
            sm_update_status(esc, n);
        }
    }
}

/* ---- SII EEPROM ------------------------------------------------------------ */

static void eeprom_command(soft_esc_t *esc)
{
    uint16_t control = rd16(&esc->mem[REG_EEP_CONTROL]);
    uint32_t i;

    if ((control & EEP_CMD_MASK) == EEP_CMD_READ)
    {
        uint32_t byteAddr = rd32(&esc->mem[REG_EEP_ADDRESS]) * 2U;

        for (i = 0U; i < 8U; i++)
        {
            uint32_t a = byteAddr + i;
            esc->mem[REG_EEP_DATA + i] = (a < SOFT_ESC_SII_MAX) ? esc->sii[a] : 0xFFU;
        }
        esc->stats.siiReads++;
    }
    /* Write/reload are not supported; every command completes at once
     * (never busy), errors cleared. */
    wr16(&esc->mem[REG_EEP_CONTROL], EEP_READ_8_BYTES);
}

/* ---- Register protection and side effects ---------------------------------- */

static bool in_range(uint32_t a, uint32_t start, uint32_t len)
{
    return (a >= start) && (a < (start + len));
}

static bool ecat_writable(uint32_t a)
{
    if (a >= REG_PRAM)
    {
        return true;
    }
    if (in_range(a, REG_TYPE, 0x10U) || in_range(a, REG_DL_STATUS, 2U)
            || in_range(a, REG_AL_STATUS, 6U) || in_range(a, REG_PDI_CONTROL, 2U)
            || in_range(a, 0x0210U, 4U) || in_range(a, REG_AL_EVENT, 4U)
            || in_range(a, REG_WD_STATUS, 2U))
    {
        return false;
    }
    if (in_range(a, REG_SM0, SOFT_ESC_NUM_SM * SM_SIZE))
    {
        uint32_t off = (a - REG_SM0) % SM_SIZE;
        return (off != SM_STATUS) && (off != SM_PDI_CONTROL);
    }
    return true;
}

static bool pdi_writable(uint32_t a)
{
    if (a >= REG_PRAM)
    {
        return true;
    }
    if (in_range(a, REG_AL_STATUS, 6U) || in_range(a, REG_AL_EVENT_MASK, 4U)
            || in_range(a, REG_AL_EVENT, 4U) || in_range(a, 0x0501U, 15U))
    {
        return true;
    }
    if (in_range(a, REG_SM0, SOFT_ESC_NUM_SM * SM_SIZE))
    {
        return ((a - REG_SM0) % SM_SIZE) == SM_PDI_CONTROL;
    }
    return false;
}

static void ecat_after_write(soft_esc_t *esc, uint32_t addr, uint32_t len)
{
    uint32_t n;

    if (overlaps(addr, len, REG_AL_CONTROL, 2U))
    {
        al_event_set(esc, AL_EVENT_CONTROL);
        esc->stats.alControlWrites++;
    }
    if (overlaps(addr, len, REG_EEP_CONTROL, 2U))
    {
        eeprom_command(esc);
    }
    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        uint32_t base = REG_SM0 + (n * SM_SIZE);

        if (overlaps(addr, len, base, SM_STATUS))
        {
            sm_reset(esc, n); /* start/length/control changed */
        }
        if (overlaps(addr, len, base + SM_ACTIVATE, 1U))
        {
            if ((esc->mem[base + SM_ACTIVATE] & 0x01U) == 0U)
            {
                sm_reset(esc, n);
            }
            al_event_set(esc, AL_EVENT_SM_ACT);
        }
    }
}

/* ---- ECAT memory access ---------------------------------------------------- */

static bool ecat_read(soft_esc_t *esc, uint32_t addr, uint8_t *dst, uint32_t len, bool orInto)
{
    uint32_t i;

    if (!sm_ecat_access(esc, addr, len, false, false))
    {
        esc->stats.denied++;
        return false;
    }
    for (i = 0U; i < len; i++)
    {
        uint32_t a = addr + i;
        uint8_t v = (a < SOFT_ESC_MEM_SIZE) ? esc->mem[a] : 0U;
        dst[i] = orInto ? (uint8_t)(dst[i] | v) : v;
    }
    (void)sm_ecat_access(esc, addr, len, false, true);
    return true;
}

static bool ecat_write(soft_esc_t *esc, uint32_t addr, const uint8_t *src, uint32_t len)
{
    uint32_t i;

    if (!sm_ecat_access(esc, addr, len, true, false))
    {
        esc->stats.denied++;
        return false;
    }
    for (i = 0U; i < len; i++)
    {
        uint32_t a = addr + i;
        if ((a < SOFT_ESC_MEM_SIZE) && ecat_writable(a))
        {
            esc->mem[a] = src[i];
        }
    }
    (void)sm_ecat_access(esc, addr, len, true, true);
    ecat_after_write(esc, addr, len);
    return true;
}

/* ---- Logical addressing through FMMUs ---------------------------------------- */

static uint16_t logical_access(soft_esc_t *esc, uint8_t cmd, uint32_t logAddr,
        uint8_t *data, uint16_t len)
{
    bool doRead = (cmd == CMD_LRD) || (cmd == CMD_LRW);
    bool doWrite = (cmd == CMD_LWR) || (cmd == CMD_LRW);
    bool readHit = false;
    bool writeHit = false;
    uint32_t n;

    if (doWrite)
    {
        (void)memcpy(esc->scratch, data, len); /* data as sent by the master */
    }

    for (n = 0U; n < SOFT_ESC_NUM_FMMU; n++)
    {
        const uint8_t *f = &esc->mem[REG_FMMU0 + (n * FMMU_SIZE)];
        uint32_t fStart;
        uint32_t fLen;
        uint32_t from;
        uint32_t to;
        uint32_t phys;

        if ((f[FMMU_ACTIVATE] & 0x01U) == 0U)
        {
            continue;
        }
        fStart = rd32(&f[FMMU_LOG_START]);
        fLen = rd16(&f[FMMU_LENGTH]);
        if (!overlaps(logAddr, len, fStart, fLen))
        {
            continue;
        }
        if ((f[FMMU_LOG_STARTBIT] != 0U) || (f[FMMU_LOG_STOPBIT] != 7U)
                || (f[FMMU_PHYS_BIT] != 0U))
        {
            esc->stats.unsupported++; /* bit-oriented mapping */
            continue;
        }

        from = (logAddr > fStart) ? logAddr : fStart;
        to = ((logAddr + len) < (fStart + fLen)) ? (logAddr + len) : (fStart + fLen);
        phys = rd16(&f[FMMU_PHYS_START]) + (from - fStart);

        if (doRead && ((f[FMMU_TYPE] & 0x01U) != 0U))
        {
            if (ecat_read(esc, phys, &data[from - logAddr], to - from, false))
            {
                readHit = true;
            }
        }
        if (doWrite && ((f[FMMU_TYPE] & 0x02U) != 0U))
        {
            if (ecat_write(esc, phys, &esc->scratch[from - logAddr], to - from))
            {
                writeHit = true;
            }
        }
    }

    return (uint16_t)((readHit ? 1U : 0U) + (writeHit ? ((cmd == CMD_LRW) ? 2U : 1U) : 0U));
}

/* ---- Datagram processing -------------------------------------------------- */

static uint16_t datagram(soft_esc_t *esc, uint8_t *hdr, uint8_t *data, uint16_t len)
{
    uint8_t cmd = hdr[0];
    uint16_t adp = rd16(&hdr[2]);
    uint16_t ado = rd16(&hdr[4]);
    bool addressed;
    bool rd = false;
    bool wr = false;
    bool orInto = false;
    uint16_t wkc = 0U;

    switch (cmd)
    {
    case CMD_APRD: case CMD_APWR: case CMD_APRW: case CMD_ARMW:
        addressed = (adp == 0U);
        wr16(&hdr[2], (uint16_t)(adp + 1U)); /* auto-increment */
        break;
    case CMD_FPRD: case CMD_FPWR: case CMD_FPRW: case CMD_FRMW:
        addressed = (adp == rd16(&esc->mem[REG_STATION_ADR]));
        break;
    case CMD_BRD: case CMD_BWR: case CMD_BRW:
        addressed = true;
        orInto = true;
        wr16(&hdr[2], (uint16_t)(adp + 1U));
        break;
    case CMD_LRD: case CMD_LWR: case CMD_LRW:
        esc->stats.addressed++;
        return logical_access(esc, cmd, (uint32_t)adp | ((uint32_t)ado << 16), data, len);
    case CMD_NOP:
        return 0U;
    default:
        esc->stats.unsupported++;
        return 0U;
    }

    if (!addressed)
    {
        return 0U;
    }
    esc->stats.addressed++;

    rd = (cmd == CMD_APRD) || (cmd == CMD_FPRD) || (cmd == CMD_BRD)
            || (cmd == CMD_APRW) || (cmd == CMD_FPRW) || (cmd == CMD_BRW)
            || (cmd == CMD_ARMW) || (cmd == CMD_FRMW);
    wr = (cmd == CMD_APWR) || (cmd == CMD_FPWR) || (cmd == CMD_BWR)
            || (cmd == CMD_APRW) || (cmd == CMD_FPRW) || (cmd == CMD_BRW);

    if (wr)
    {
        (void)memcpy(esc->scratch, data, len);
    }
    if (rd && ecat_read(esc, ado, data, len, orInto))
    {
        wkc++;
    }
    if (wr && ecat_write(esc, ado, esc->scratch, len))
    {
        wkc = (uint16_t)(wkc + (rd ? 2U : 1U));
    }
    return wkc;
}

bool soft_esc_process_frame(soft_esc_t *esc, uint8_t *frame, uint16_t len)
{
    uint16_t ecatLen;
    uint8_t *p;
    uint8_t *end;

    if ((esc == NULL) || (frame == NULL)
            || (len < (ETH_HEADER_LEN + ECAT_HEADER_LEN + DG_HEADER_LEN + DG_WKC_LEN)))
    {
        return false;
    }
    if ((((uint16_t)frame[12] << 8) | frame[13]) != ETH_TYPE_ECAT)
    {
        return false;
    }
    ecatLen = rd16(&frame[ETH_HEADER_LEN]);
    if ((ecatLen >> 12) != 1U) /* type 1: EtherCAT commands */
    {
        return false;
    }

    p = &frame[ETH_HEADER_LEN + ECAT_HEADER_LEN];
    end = p + (ecatLen & DG_LEN_MASK);
    if (end > &frame[len])
    {
        end = &frame[len];
    }

    esc->stats.frames++;
    while ((p + DG_HEADER_LEN + DG_WKC_LEN) <= end)
    {
        uint16_t lenField = rd16(&p[6]);
        uint16_t dlen = (uint16_t)(lenField & DG_LEN_MASK);
        uint8_t *data = &p[DG_HEADER_LEN];
        uint8_t *wkc = data + dlen;

        if (((wkc + DG_WKC_LEN) > end) || (dlen > SOFT_ESC_MAX_DATAGRAM))
        {
            break;
        }
        esc->stats.datagrams++;
        wr16(wkc, (uint16_t)(rd16(wkc) + datagram(esc, p, data, dlen)));

        p = wkc + DG_WKC_LEN;
        if ((lenField & DG_MORE) == 0U)
        {
            break;
        }
    }

    frame[6] |= 0x02U; /* source MAC U/L bit: frame passed an ESC */
    return true;
}

/* ---- PDI access ------------------------------------------------------------- */

void soft_esc_pdi_read(soft_esc_t *esc, uint16_t addr, void *buf, uint16_t len)
{
    uint8_t *dst = buf;
    uint32_t i;
    uint32_t n;

    if ((esc == NULL) || (buf == NULL))
    {
        return;
    }
    for (i = 0U; i < len; i++)
    {
        uint32_t a = (uint32_t)addr + i;
        dst[i] = (a < SOFT_ESC_MEM_SIZE) ? esc->mem[a] : 0U;
    }

    if (overlaps(addr, len, REG_AL_CONTROL, 2U))
    {
        al_event_clear(esc, AL_EVENT_CONTROL);
    }
    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        if (overlaps(addr, len, REG_SM0 + (n * SM_SIZE) + SM_ACTIVATE, 1U))
        {
            al_event_clear(esc, AL_EVENT_SM_ACT);
        }
    }
    if (addr >= REG_PRAM)
    {
        sm_pdi_access(esc, addr, len, false);
    }
}

void soft_esc_pdi_write(soft_esc_t *esc, uint16_t addr, const void *buf, uint16_t len)
{
    const uint8_t *src = buf;
    uint32_t i;
    uint32_t n;

    if ((esc == NULL) || (buf == NULL))
    {
        return;
    }
    for (i = 0U; i < len; i++)
    {
        uint32_t a = (uint32_t)addr + i;
        if ((a < SOFT_ESC_MEM_SIZE) && pdi_writable(a))
        {
            esc->mem[a] = src[i];
        }
    }

    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        uint32_t pdiCtl = REG_SM0 + (n * SM_SIZE) + SM_PDI_CONTROL;

        if (overlaps(addr, len, pdiCtl, 1U) && ((esc->mem[pdiCtl] & 0x01U) != 0U))
        {
            sm_reset(esc, n); /* SM deactivated by the PDI */
        }
    }
    if (addr >= REG_PRAM)
    {
        sm_pdi_access(esc, addr, len, true);
    }
}

/* ---- Init ------------------------------------------------------------------- */

void soft_esc_init(soft_esc_t *esc, const uint8_t *sii, uint16_t siiBytes)
{
    uint32_t n;

    if (esc == NULL)
    {
        return;
    }
    (void)memset(esc, 0, sizeof(*esc));
    (void)memset(esc->sii, 0xFF, sizeof(esc->sii));
    if (sii != NULL)
    {
        (void)memcpy(esc->sii, sii,
                (siiBytes < SOFT_ESC_SII_MAX) ? siiBytes : SOFT_ESC_SII_MAX);
    }

    esc->mem[REG_TYPE] = ESC_TYPE_VALUE;
    esc->mem[REG_REVISION] = 0x01U;
    wr16(&esc->mem[REG_BUILD], 0x0001U);
    esc->mem[REG_FMMU_NUM] = (uint8_t)SOFT_ESC_NUM_FMMU;
    esc->mem[REG_SM_NUM] = (uint8_t)SOFT_ESC_NUM_SM;
    esc->mem[REG_RAM_SIZE] = (uint8_t)((SOFT_ESC_MEM_SIZE - REG_PRAM) / 1024U);
    esc->mem[REG_PORT_DESC] = 0x0FU;        /* ports 0 and 1 (MII) */
    wr16(&esc->mem[REG_FEATURES], 0x0000U); /* no DC */
    wr16(&esc->mem[REG_DL_STATUS], DL_STATUS_VALUE);
    wr16(&esc->mem[REG_AL_STATUS], 0x0001U); /* INIT */
    esc->mem[REG_PDI_CONTROL] = PDI_TYPE_ONCHIP;
    wr16(&esc->mem[REG_WD_STATUS], 0x0001U); /* process data watchdog ok */
    wr16(&esc->mem[REG_EEP_CONTROL], EEP_READ_8_BYTES);

    /* SII word 4 = configured station alias */
    esc->mem[0x0012U] = esc->sii[8];
    esc->mem[0x0013U] = esc->sii[9];

    for (n = 0U; n < SOFT_ESC_NUM_SM; n++)
    {
        sm_update_status(esc, n);
    }
}
