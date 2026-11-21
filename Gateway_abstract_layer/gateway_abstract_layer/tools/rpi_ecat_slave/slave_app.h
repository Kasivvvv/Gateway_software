#ifndef SLAVE_APP_H
#define SLAVE_APP_H

#include <stdint.h>
#include <stdbool.h>

#include "soft_esc.h"

/*
 * EtherCAT test slave: SOES on the software ESC, object dictionary and SII
 * image of the SOES "linux_lan9252demo" application (vendor 0x1337, product
 * 1234): outputs LED0, LED1 (2 bytes), input Button1 (1 byte).
 * Test behaviour: Button1 = LED0 + 1, so a counter written by the master
 * comes back incremented in its inputs.
 */

/** Initialise the soft-ESC with the SII image and start the SOES stack. */
void slave_app_init(uint32_t watchdogCalls);

/** Run the SOES stack once (state machine, mailbox, process data). */
void slave_app_poll(void);

/** The single soft-ESC instance (ECAT side for the frame loop). */
soft_esc_t *slave_app_esc(void);

uint8_t slave_app_al_status(void);
uint32_t slave_app_outputs_updates(void);

#endif /* SLAVE_APP_H */
