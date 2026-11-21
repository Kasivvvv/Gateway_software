#ifndef SOFT_ESC_H
#define SOFT_ESC_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Software EtherCAT Slave Controller (soft-ESC).
 *
 * Emulates the parts of an ESC (Beckhoff ET1100 / Microchip LAN9252 register
 * model, ETG.1000.4) that SOEM (master) and SOES (slave stack) rely on:
 *   - ECAT side: in-place processing of an EtherCAT frame (all datagram
 *     types except DC-specific behaviour), working counter, auto-increment /
 *     configured / broadcast / logical (FMMU) addressing;
 *   - PDI side: register/RAM read and write for the slave stack (SOES HAL);
 *   - SyncManagers: mailbox (full/empty handshake) and buffered mode (one
 *     buffer, no 3-buffer swapping), AL event request bits;
 *   - AL control/status handshake, SM activation events;
 *   - SII EEPROM interface (read, 8-byte chunks) backed by a RAM image.
 * Not emulated: Distributed Clocks, watchdog timers, bit-oriented FMMU,
 * multiple ports/forwarding (the slave is the last one: port 0 only).
 *
 * Used by the RPi slave (configurations 3/4) and later by the gateway itself
 * as EtherCAT slave over GMAC (configurations 1/2). Single-threaded: the
 * caller serialises ECAT and PDI access.
 */

#define SOFT_ESC_MEM_SIZE     (0x2000U)  /* 4 KB registers + 4 KB process RAM */
#define SOFT_ESC_NUM_SM       (8U)
#define SOFT_ESC_NUM_FMMU     (8U)
#define SOFT_ESC_SII_MAX      (2048U)    /* bytes of SII EEPROM image */
#define SOFT_ESC_MAX_DATAGRAM (1486U)

typedef struct
{
    uint32_t frames;          /* EtherCAT frames processed */
    uint32_t datagrams;
    uint32_t addressed;       /* datagrams that touched this slave */
    uint32_t denied;          /* SyncManager accesses refused (no WKC) */
    uint32_t siiReads;
    uint32_t alControlWrites;
    uint32_t unsupported;     /* bit-oriented FMMU or unknown commands */
} soft_esc_stats_t;

typedef struct
{
    uint8_t  mem[SOFT_ESC_MEM_SIZE];
    uint8_t  sii[SOFT_ESC_SII_MAX];
    bool     smFull[SOFT_ESC_NUM_SM];        /* mailbox SM: buffer full */
    uint8_t  scratch[SOFT_ESC_MAX_DATAGRAM]; /* original data of RW datagrams */
    soft_esc_stats_t stats;
} soft_esc_t;

/** Reset registers to power-on values and load the SII image. */
void soft_esc_init(soft_esc_t *esc, const uint8_t *sii, uint16_t siiBytes);

/**
 * ECAT side: process one received Ethernet frame in place (datagrams,
 * working counters, auto-increment addresses). Returns false if the frame is
 * not an EtherCAT command frame (then it is left unchanged).
 */
bool soft_esc_process_frame(soft_esc_t *esc, uint8_t *frame, uint16_t len);

/** PDI side (slave stack): read/write registers and process RAM. */
void soft_esc_pdi_read(soft_esc_t *esc, uint16_t addr, void *buf, uint16_t len);
void soft_esc_pdi_write(soft_esc_t *esc, uint16_t addr, const void *buf, uint16_t len);

#endif /* SOFT_ESC_H */
