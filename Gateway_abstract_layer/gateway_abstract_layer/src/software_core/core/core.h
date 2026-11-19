#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize application core after peripheral SYS_Initialize().
 *
 * Sets up software modules that do not depend on MCC-generated init.
 * Stage 0: LED0 blink smoke test only.
 */
void core_init(void);

/**
 * @brief One non-blocking iteration of the gateway superloop.
 *
 * Called once per main-loop cycle. Must not busy-wait.
 */
void core_step(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
