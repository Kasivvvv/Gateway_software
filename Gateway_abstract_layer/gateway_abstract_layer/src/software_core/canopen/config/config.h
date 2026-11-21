/**
 * @file config.h
 * @brief Compile-time configuration for lely-core (N7S) on SAM V71 bare-metal.
 *
 * Included when HAVE_CONFIG_H is defined. Node (stage 3) and NMT master
 * (stage 7) roles, no malloc, classic CAN, RPDO/TPDO/SYNC/SSDO only.
 */
#ifndef GATEWAY_LELY_CONFIG_H_
#define GATEWAY_LELY_CONFIG_H_

/* Memory / environment */
#define LELY_NO_MALLOC 1
#define LELY_NO_THREADS 1
#define LELY_NO_ATOMICS 1
#define LELY_NO_STDIO 1
#define LELY_NO_DIAG 1
#define LELY_NO_CXX 1
#define LELY_NO_DAEMON 1
#define LELY_NO_CANFD 1

/* XC32 musl already provides these; stop lely from redefining them. */
#define LELY_HAVE_TIMESPEC 1
#define LELY_HAVE_ITIMERSPEC 1

/* CANopen feature set (gateway slave for Stage 3 / set S1) */
#define LELY_NO_CO_DCF 1
#define LELY_NO_CO_DCF_RESTORE 1
#define LELY_NO_CO_SDEV 1
#define LELY_NO_CO_CSDO 1
#define LELY_NO_CO_TIME 1
#define LELY_NO_CO_EMCY 1
#define LELY_NO_CO_LSS 1
#define LELY_NO_CO_WTM 1
/* LELY_NO_CO_MASTER not defined: NMT master (configurations 2 and 4, stage 7);
 * the slave boot-up process stays disabled (NMT_BOOT, NMT_CFG, CSDO), the
 * gateway starts and stops the nodes itself. */
#define LELY_NO_CO_NG 1
#define LELY_NO_CO_NMT_BOOT 1
#define LELY_NO_CO_NMT_CFG 1
#define LELY_NO_CO_GW 1
#define LELY_NO_CO_GW_TXT 1
#define LELY_NO_CO_ECSS_REDUNDANCY 1
#define LELY_NO_COAPP_MASTER 1
#define LELY_NO_COAPP_SLAVE 1

/* Shrink static OD footprint (dcf2dev --no-strings) */
#define LELY_NO_CO_OBJ_NAME 1
#define LELY_NO_CO_OBJ_FILE 1

/* Keep RPDO / TPDO / SYNC / SSDO enabled (do not define LELY_NO_CO_* for them). */

#endif /* GATEWAY_LELY_CONFIG_H_ */
