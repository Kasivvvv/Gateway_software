/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    Thin Harmony APP glue. Gateway logic lives in software_core/core.

  Description:
    main() calls core_init/core_step directly. APP_* remains for Harmony
    compatibility if SYS_Tasks() is restored; it forwards to the same core.
 *******************************************************************************/

#include "app.h"
#include "core.h"

APP_DATA appData;

void APP_Initialize(void)
{
    appData.state = APP_STATE_INIT;
    core_init();
}

void APP_Tasks(void)
{
    switch (appData.state)
    {
        case APP_STATE_INIT:
            appData.state = APP_STATE_SERVICE_TASKS;
            break;

        case APP_STATE_SERVICE_TASKS:
            core_step();
            break;

        default:
            break;
    }
}

/*******************************************************************************
 End of File
 */
