#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Supervisory state machine of the gateway (thesis 7.3, Table 7.1 states,
 * Table 7.2 transitions T1..T11, plus T12 Safe -> Error). Pure logic:
 * evaluated once per superloop
 * iteration (T_dec <= Tloop); the Software Core collects the inputs and
 * executes the commands.
 */

typedef enum
{
    GW_ST_INIT = 0,
    GW_ST_STARTUP,
    GW_ST_CONFIGURATION,
    GW_ST_WORKING,
    GW_ST_ERROR,
    GW_ST_SAFE
} gw_state_t;

/* Error class determined by the Error handler; selects the exit of the
 * Error State (hierarchy of recovery, thesis 7.3). */
typedef enum
{
    GW_ERR_CLASS_NONE = 0,
    GW_ERR_CLASS_RECOVERABLE,  /* T9: resume without re-initialisation */
    GW_ERR_CLASS_REINIT,       /* T10: re-initialise the communication */
    GW_ERR_CLASS_FATAL         /* T11: hardware reset, last resort */
} gw_err_class_t;

typedef struct
{
    bool           lowLevelInitDone;  /* T1 */
    bool           selfTestDone;      /* T2 / T3 */
    bool           selfTestOk;
    bool           stacksReady;       /* T4 */
    bool           stacksFailed;      /* T5 */
    gw_err_class_t errorClass;        /* active error (Error handler), T6 */
    bool           errorReset;        /* Error handler: cause removed, T9 */
    bool           safeCause;         /* safe-state request / link loss, T7 */
    bool           resume;            /* resume command received, T8 */
} sm_inputs_t;

typedef struct
{
    gw_state_t state;
    gw_state_t previous;
    uint8_t    transition;   /* 1..12 if a transition happened in this step */
    bool       convert;      /* working state command: conversion permitted */
    bool       commStop;     /* communication stop command (entry of Error/Safe) */
    bool       commReset;    /* communication reset command (T10) */
    bool       hwReset;      /* hardware reset (T11) */
} sm_output_t;

void sm_init(void);
void sm_step(const sm_inputs_t *in, sm_output_t *out);
gw_state_t sm_state(void);
const char *sm_state_name(gw_state_t st);

#endif /* STATE_MACHINE_H */
