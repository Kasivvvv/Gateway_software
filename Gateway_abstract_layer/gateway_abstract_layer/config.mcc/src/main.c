/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Application entry: peripheral init, then gateway core superloop.

  Description:
    SYS_Initialize() brings up MCC/Harmony peripherals (clocks, GPIO, TC0/SYS_TIME,
    etc.). The application then runs software_core::core_step() in a bare-metal
    superloop. Polled Harmony stacks (TCP/IP, MIIM) are intentionally not pumped
    here — they will be removed from MCC; EtherCAT will use a raw GMAC driver.
 *******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include "core.h"

int main(void)
{
    /* Initialize MCU peripherals and Harmony modules (MCC-generated). */
    SYS_Initialize(NULL);

    /* Application core (software_core). */
    core_init();

    while (true)
    {
        core_step();
    }

    return (EXIT_FAILURE);
}

/*******************************************************************************
 End of File
 */
