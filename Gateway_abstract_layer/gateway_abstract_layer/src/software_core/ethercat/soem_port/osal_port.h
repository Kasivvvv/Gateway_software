#ifndef OSAL_PORT_H
#define OSAL_PORT_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Gateway extension of the SOEM OSAL: a global deadline for blocking SOEM
 * calls. While a deadline is set and reached, every SOEM timer reports
 * "expired" and osal_usleep() returns at once, so any blocking SOEM function
 * finishes (with a failure) shortly after the deadline. This enforces the
 * time limit of the Configuration state without modifying SOEM.
 */

/** Set an absolute deadline (hwa_time_now_us() base); 0 = no deadline. */
void osal_deadline_set(uint64_t deadline_us);

/** True if the deadline was reached since it was set. */
bool osal_deadline_hit(void);

#endif /* OSAL_PORT_H */
