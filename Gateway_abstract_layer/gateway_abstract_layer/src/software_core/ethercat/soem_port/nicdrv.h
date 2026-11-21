/*
 * SOEM NIC driver interface for the bare-metal gateway (GMAC via hwa_eth_*).
 * Same structure as SOEM oshw/<os>/nicdrv.h (field names are used by the
 * SOEM core). Single port, no cable redundancy, no locking (superloop only).
 */
#ifndef _nicdrvh_
#define _nicdrvh_

#ifdef __cplusplus
extern "C"
{
#endif

/** pointer structure to Tx and Rx stacks */
typedef struct
{
    int     *sock;
    ec_bufT (*txbuf)[EC_MAXBUF];
    int     (*txbuflength)[EC_MAXBUF];
    ec_bufT *tempbuf;
    ec_bufT (*rxbuf)[EC_MAXBUF];
    int     (*rxbufstat)[EC_MAXBUF];
    int     (*rxsa)[EC_MAXBUF];
} ec_stackT;

/** pointer structure to buffers for redundant port (unused, kept for API) */
typedef struct
{
    ec_stackT stack;
    int       sockhandle;
    ec_bufT   rxbuf[EC_MAXBUF];
    int       rxbufstat[EC_MAXBUF];
    int       rxsa[EC_MAXBUF];
    ec_bufT   tempinbuf;
} ecx_redportt;

/** pointer structure to buffers, vars and mutexes for port instantiation */
typedef struct
{
    ec_stackT     stack;
    int           sockhandle;
    ec_bufT       rxbuf[EC_MAXBUF];
    int           rxbufstat[EC_MAXBUF];
    int           rxsa[EC_MAXBUF];
    ec_bufT       tempinbuf;
    int           tempinbufs;
    ec_bufT       txbuf[EC_MAXBUF];
    int           txbuflength[EC_MAXBUF];
    ec_bufT       txbuf2;
    int           txbuflength2;
    int           lastidx;
    int           redstate;
    ecx_redportt *redport;
} ecx_portt;

extern const uint16 priMAC[3];
extern const uint16 secMAC[3];

void ec_setupheader(void *p);
int ecx_setupnic(ecx_portt *port, const char *ifname, int secondary);
int ecx_closenic(ecx_portt *port);
void ecx_setbufstat(ecx_portt *port, int idx, int bufstat);
int ecx_getindex(ecx_portt *port);
int ecx_outframe(ecx_portt *port, int idx, int sock);
int ecx_outframe_red(ecx_portt *port, int idx);
int ecx_waitinframe(ecx_portt *port, int idx, int timeout);
int ecx_srconfirm(ecx_portt *port, int idx, int timeout);
int ecx_inframe(ecx_portt *port, int idx, int stacknumber);

#ifdef __cplusplus
}
#endif

#endif /* _nicdrvh_ */
