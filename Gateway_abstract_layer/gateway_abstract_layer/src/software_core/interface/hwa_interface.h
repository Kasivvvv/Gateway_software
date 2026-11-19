#ifndef HWA_INTERFACE_H
#define HWA_INTERFACE_H

#include <stdint.h>

void hwa_init(void);
void hwa_deinit(void);


// CAN -------------------------------------------------------------------------------------*/
void hwa_can_tx(uint32_t id, uint32_t sub_id, uint8_t *data, uint8_t len);
void hwa_can_rx(uint32_t *id, uint32_t *sub_id, uint8_t *data, uint8_t *len);
void hwa_get_number_of_massages(void);

// ETHERCAT ---------------------------------------------------------------------------------*/


// USB --------------------------------------------------------------------------------------*/



#endif // HWA_INTERFACE_H   