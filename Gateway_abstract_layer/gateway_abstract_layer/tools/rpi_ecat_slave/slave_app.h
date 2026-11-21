#ifndef SLAVE_APP_H
#define SLAVE_APP_H

#include <stdint.h>
#include <stdbool.h>

#include "soft_esc.h"

/*
 * EtherCAT test slave: SOES on the software ESC. SII image of the SOES
 * "linux_lan9252demo" (vendor 0x1337, product 1234), own object dictionary
 * (od/): 8 bytes of outputs (0x7000:01) and 8 bytes of inputs (0x6000:01).
 * Test behaviour: inputs = outputs (echo), so data written by the master
 * come back unchanged in its inputs.
 */

/** Initialise the soft-ESC with the SII image and start the SOES stack. */
void slave_app_init(uint32_t watchdogCalls);

/** Run the SOES stack once (state machine, mailbox, process data). */
void slave_app_poll(void);

/** The single soft-ESC instance (ECAT side for the frame loop). */
soft_esc_t *slave_app_esc(void);

uint8_t slave_app_al_status(void);
uint32_t slave_app_outputs_updates(void);
uint64_t slave_app_outputs(void);
uint64_t slave_app_inputs(void);
/** Second 8-byte entry of the outputs (node 2 in configuration 4). */
uint64_t slave_app_outputs2(void);

#endif /* SLAVE_APP_H */
