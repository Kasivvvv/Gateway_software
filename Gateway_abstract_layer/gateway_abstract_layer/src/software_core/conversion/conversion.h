#ifndef CONVERSION_H
#define CONVERSION_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Conversion module (thesis 7.2.5): buffers between the CANopen and the
 * EtherCAT side, one per direction.
 *
 *   push : insertion of a converted message into the buffer   -> marker t2
 *   pop  : removal from the buffer (start of its service)     -> marker t3
 *          the message is returned when its service has ended (t4 follows
 *          in the output driver)
 *
 * Capacity K counts every message in the system of a direction, including
 * the one in service, as in the M/M/1/K model (thesis 5.2.4, Eq. 5.7). A push
 * into a full buffer is lost and counted as an overflow (conversion fault,
 * transition T6).
 *
 * Strategy A (immediate forwarding) is implemented; B and C follow in
 * stage 7. The service modes other than IMMEDIATE are the test mode of
 * HIL-03: an extended deterministic or exponential service time.
 *
 * Pure logic: no hardware access except the t2/t3 markers via hwa_marker.
 */

#define CONV_K_MAX     (64U)
#define CONV_DATA_MAX  (8U)

typedef enum
{
    CONV_DIR_CAN_TO_EC = 0,
    CONV_DIR_EC_TO_CAN,
    CONV_DIR_COUNT
} conv_dir_t;

typedef enum
{
    CONV_STRATEGY_A = 0   /* immediate forwarding */
} conv_strategy_t;

typedef enum
{
    CONV_SERVICE_IMMEDIATE = 0,
    CONV_SERVICE_DETERMINISTIC,   /* HIL-03 test mode */
    CONV_SERVICE_EXPONENTIAL      /* HIL-03 test mode */
} conv_service_t;

typedef struct
{
    uint8_t  data[CONV_DATA_MAX];
    uint8_t  len;
    uint16_t source;     /* e.g. CAN identifier or EtherCAT slave */
    uint32_t seq;        /* assigned by push */
    uint64_t t2Us;       /* insertion time */
} conv_msg_t;

typedef struct
{
    uint32_t       capacity;       /* K, 1..CONV_K_MAX */
    conv_service_t service;
    uint32_t       serviceMeanUs;  /* mean service time in test mode */
} conv_dir_config_t;

typedef struct
{
    conv_strategy_t   strategy;
    conv_dir_config_t dir[CONV_DIR_COUNT];
} conv_config_t;

typedef struct
{
    uint32_t inserted;
    uint32_t removed;
    uint32_t overflows;
    uint32_t fill;        /* messages in the system now (queue + service) */
    uint32_t highWater;
} conv_stats_t;

/** Configure and empty both buffers; false if the configuration is invalid. */
bool conversion_init(const conv_config_t *cfg);

/** Drop all buffered messages (conversion stopped); statistics are kept. */
void conversion_flush(void);

/** Insert a message (t2). False if the buffer is full (overflow). */
bool conversion_push(conv_dir_t dir, const conv_msg_t *msg, uint64_t nowUs);

/** Take the next message whose service has ended (t3 at removal). */
bool conversion_pop(conv_dir_t dir, conv_msg_t *msg, uint64_t nowUs);

void conversion_stats_get(conv_dir_t dir, conv_stats_t *stats);

/** Conversion status for the state machine: an overflow occurred. */
bool conversion_fault(void);
void conversion_fault_clear(void);

#endif /* CONVERSION_H */
