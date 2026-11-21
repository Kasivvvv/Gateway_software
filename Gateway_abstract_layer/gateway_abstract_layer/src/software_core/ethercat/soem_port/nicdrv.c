/*
 * SOEM NIC driver for the bare-metal gateway: raw frames through hwa_eth_*.
 *
 * Based on the structure of SOEM v1.4.0 oshw/erika/nicdrv.c (GPLv2 with the
 * SOEM exception, see third_party/SOEM/LICENSE). Differences:
 *   - frames go to/come from hwa_eth_tx()/hwa_eth_rx() (GMAC, hal_eth.c);
 *   - single port only (no cable redundancy), no locking: every call comes
 *     from the superloop context;
 *   - ecx_inframe() reads a bounded number of frames per call, so a busy
 *     non-EtherCAT network cannot keep it looping.
 *
 * Blocking behaviour is unchanged: ecx_srconfirm()/ecx_waitinframe() poll
 * until their timeout. The gateway calls them only in the Configuration state;
 * in the Working state it uses timeout 0 (one poll, non-blocking).
 */
#include <string.h>

#include "oshw.h"
#include "osal.h"
#include "nicdrv.h"

#include "hwa_interface.h"

/* Frames inspected per ecx_inframe() call before giving up. */
#define NICDRV_RX_MAX_PER_CALL (16)

enum
{
    ECT_RED_NONE,
    ECT_RED_DOUBLE
};

/** Primary source MAC address used for EtherCAT (as in SOEM). */
const uint16 priMAC[3] = { 0x0201, 0x0101, 0x0101 };
/** Secondary source MAC address used for EtherCAT (unused here). */
const uint16 secMAC[3] = { 0x0604, 0x0404, 0x0404 };

static void ecx_clear_rxbufstat(int *rxbufstat)
{
    int i;

    for (i = 0; i < EC_MAXBUF; i++)
    {
        rxbufstat[i] = EC_BUF_EMPTY;
    }
}

int ecx_setupnic(ecx_portt *port, const char *ifname, int secondary)
{
    int i;

    (void)ifname; /* single interface: the on-board GMAC */

    if (secondary)
    {
        return 0; /* no redundant port on this board */
    }

    port->sockhandle = -1;
    port->lastidx = 0;
    port->redstate = ECT_RED_NONE;
    port->redport = NULL;
    port->stack.sock = &(port->sockhandle);
    port->stack.txbuf = &(port->txbuf);
    port->stack.txbuflength = &(port->txbuflength);
    port->stack.tempbuf = &(port->tempinbuf);
    port->stack.rxbuf = &(port->rxbuf);
    port->stack.rxbufstat = &(port->rxbufstat);
    port->stack.rxsa = &(port->rxsa);
    ecx_clear_rxbufstat(&(port->rxbufstat[0]));

    /* Ethernet headers are written once; SOEM only changes the payload. */
    for (i = 0; i < EC_MAXBUF; i++)
    {
        ec_setupheader(&(port->txbuf[i]));
        port->rxbufstat[i] = EC_BUF_EMPTY;
    }
    ec_setupheader(&(port->txbuf2));

    return 1;
}

int ecx_closenic(ecx_portt *port)
{
    (void)port;
    return 0;
}

void ec_setupheader(void *p)
{
    ec_etherheadert *bp = p;

    bp->da0 = oshw_htons(0xffff);
    bp->da1 = oshw_htons(0xffff);
    bp->da2 = oshw_htons(0xffff);
    bp->sa0 = oshw_htons(priMAC[0]);
    bp->sa1 = oshw_htons(priMAC[1]);
    bp->sa2 = oshw_htons(priMAC[2]);
    bp->etype = oshw_htons(ETH_P_ECAT);
}

int ecx_getindex(ecx_portt *port)
{
    int idx;
    int cnt = 0;

    idx = port->lastidx + 1;
    if (idx >= EC_MAXBUF)
    {
        idx = 0;
    }
    /* find an unused index */
    while ((port->rxbufstat[idx] != EC_BUF_EMPTY) && (cnt < EC_MAXBUF))
    {
        idx++;
        cnt++;
        if (idx >= EC_MAXBUF)
        {
            idx = 0;
        }
    }
    port->rxbufstat[idx] = EC_BUF_ALLOC;
    port->lastidx = idx;
    return idx;
}

void ecx_setbufstat(ecx_portt *port, int idx, int bufstat)
{
    port->rxbufstat[idx] = bufstat;
}

int ecx_outframe(ecx_portt *port, int idx, int stacknumber)
{
    int lp;

    if (stacknumber != 0)
    {
        return -1;
    }

    lp = port->txbuflength[idx];
    port->rxbufstat[idx] = EC_BUF_TX;
    if (!hwa_eth_tx((const uint8_t *)&(port->txbuf[idx]), (uint16_t)lp))
    {
        return -1; /* link down or TX ring full; SOEM retries on timeout */
    }
    return lp;
}

int ecx_outframe_red(ecx_portt *port, int idx)
{
    ec_etherheadert *ehp = (ec_etherheadert *)&(port->txbuf[idx]);

    /* rewrite MAC source address 1 to primary */
    ehp->sa1 = oshw_htons(priMAC[1]);
    return ecx_outframe(port, idx, 0);
}

/* Non-blocking: one frame from the GMAC RX ring into tempinbuf. */
static int ecx_recvpkt(ecx_portt *port)
{
    uint16_t len = hwa_eth_rx((uint8_t *)&(port->tempinbuf),
            (uint16_t)sizeof(port->tempinbuf));

    port->tempinbufs = len;
    return (len > 0U) ? 1 : 0;
}

/* Copy a received EtherCAT frame (without Ethernet header) into rxbuf. */
static void ecx_store(ecx_portt *port, int idx, int lenOnWire)
{
    int copy = port->txbuflength[idx] - ETH_HEADERSIZE;
    int avail = lenOnWire - ETH_HEADERSIZE;

    if (copy > avail)
    {
        copy = avail;
    }
    if (copy > 0)
    {
        memcpy(&(port->rxbuf[idx]), &(port->tempinbuf[ETH_HEADERSIZE]), (size_t)copy);
    }
}

int ecx_inframe(ecx_portt *port, int idx, int stacknumber)
{
    uint16 l;
    int rval = EC_NOFRAME;
    int n;
    ec_bufT *rxbuf;

    if (stacknumber != 0)
    {
        return EC_NOFRAME;
    }

    rxbuf = &(port->rxbuf[idx]);

    /* requested index already received while waiting for another one? */
    if ((idx < EC_MAXBUF) && (port->rxbufstat[idx] == EC_BUF_RCVD))
    {
        l = (*rxbuf)[0] + ((uint16)((*rxbuf)[1] & 0x0f) << 8);
        rval = ((*rxbuf)[l] + ((uint16)(*rxbuf)[l + 1] << 8));
        port->rxbufstat[idx] = EC_BUF_COMPLETE;
        return rval;
    }

    for (n = 0; n < NICDRV_RX_MAX_PER_CALL; n++)
    {
        ec_etherheadert *ehp;
        ec_comt *ecp;
        int idxf;

        if (!ecx_recvpkt(port))
        {
            break; /* RX ring empty */
        }

        rval = EC_OTHERFRAME;
        ehp = (ec_etherheadert *)&(port->tempinbuf);
        if ((port->tempinbufs < (ETH_HEADERSIZE + EC_HEADERSIZE))
                || (ehp->etype != oshw_htons(ETH_P_ECAT)))
        {
            continue; /* not EtherCAT: ignore */
        }

        ecp = (ec_comt *)(&(port->tempinbuf[ETH_HEADERSIZE]));
        l = etohs(ecp->elength) & 0x0fff;
        idxf = ecp->index;

        if (idxf == idx)
        {
            ecx_store(port, idx, port->tempinbufs);
            rval = ((*rxbuf)[l] + ((uint16)((*rxbuf)[l + 1]) << 8));
            port->rxbufstat[idx] = EC_BUF_COMPLETE;
            port->rxsa[idx] = oshw_ntohs(ehp->sa1);
            break;
        }
        if ((idxf < EC_MAXBUF) && (port->rxbufstat[idxf] == EC_BUF_TX))
        {
            /* frame for another pending index: keep it for later */
            ecx_store(port, idxf, port->tempinbufs);
            port->rxbufstat[idxf] = EC_BUF_RCVD;
            port->rxsa[idxf] = oshw_ntohs(ehp->sa1);
        }
        /* else: late or foreign EtherCAT frame, dropped */
    }

    /* WKC if the matching frame was found, else EC_NOFRAME/EC_OTHERFRAME */
    return (rval >= 0) ? rval : EC_NOFRAME;
}

void nicdrv_rx_poll(ecx_portt *port)
{
    int n;

    for (n = 0; n < NICDRV_RX_MAX_PER_CALL; n++)
    {
        ec_etherheadert *ehp;
        ec_comt *ecp;
        int idxf;

        if (!ecx_recvpkt(port))
        {
            break;
        }
        ehp = (ec_etherheadert *)&(port->tempinbuf);
        if ((port->tempinbufs < (ETH_HEADERSIZE + EC_HEADERSIZE))
                || (ehp->etype != oshw_htons(ETH_P_ECAT)))
        {
            continue;
        }
        ecp = (ec_comt *)(&(port->tempinbuf[ETH_HEADERSIZE]));
        idxf = ecp->index;
        if ((idxf < EC_MAXBUF) && (port->rxbufstat[idxf] == EC_BUF_TX))
        {
            ecx_store(port, idxf, port->tempinbufs);
            port->rxbufstat[idxf] = EC_BUF_RCVD;
            port->rxsa[idxf] = oshw_ntohs(ehp->sa1);
        }
    }
}

int ecx_waitinframe(ecx_portt *port, int idx, int timeout)
{
    int wkc;
    osal_timert timer;

    osal_timer_start(&timer, (uint32)timeout);
    do
    {
        wkc = ecx_inframe(port, idx, 0);
    } while ((wkc <= EC_NOFRAME) && !osal_timer_is_expired(&timer));

    return wkc;
}

int ecx_srconfirm(ecx_portt *port, int idx, int timeout)
{
    int wkc = EC_NOFRAME;
    osal_timert timer1;

    osal_timer_start(&timer1, (uint32)timeout);
    do
    {
        osal_timert timer2;

        ecx_outframe_red(port, idx);
        osal_timer_start(&timer2,
                (uint32)((timeout < EC_TIMEOUTRET) ? timeout : EC_TIMEOUTRET));
        do
        {
            wkc = ecx_inframe(port, idx, 0);
        } while ((wkc <= EC_NOFRAME) && !osal_timer_is_expired(&timer2));
    } while ((wkc <= EC_NOFRAME) && !osal_timer_is_expired(&timer1));

    return wkc;
}
