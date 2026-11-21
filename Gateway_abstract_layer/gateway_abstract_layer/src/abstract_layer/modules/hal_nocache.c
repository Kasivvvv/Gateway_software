#include "hal_interface.h"
#include "hal_nocache.h"

#include "definitions.h"

_Static_assert((HAL_NOCACHE_MCAN_ADDR == HAL_NOCACHE_BASE)
        && ((HAL_NOCACHE_MCAN_ADDR + HAL_NOCACHE_MCAN_MAX) <= HAL_NOCACHE_GMAC_ADDR)
        && (HAL_NOCACHE_GMAC_ADDR < HAL_NOCACHE_END)
        && ((HAL_NOCACHE_BASE % HAL_NOCACHE_SIZE) == 0UL),
        "inconsistent non-cacheable window layout");

/* RASR.TEX/C/B = 001/0/0: Normal memory, non-cacheable (ARMv7-M ARM, B3.5). */
#define HAL_NOCACHE_TEX_NORMAL_NC  (1UL)

/*
 * The highest-numbered enabled MPU region that touches the window decides its
 * attributes (ARMv7-M: higher region number takes priority on overlap). It
 * must cover the whole window and be Normal, non-cacheable.
 */
bool hal_nocache_region_ok(void)
{
    uint32_t regions;
    int32_t r;

    if ((MPU->CTRL & MPU_CTRL_ENABLE_Msk) == 0U)
    {
        return false; /* default map: SRAM is write-back cacheable */
    }

    regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

    for (r = (int32_t)regions - 1; r >= 0; r--)
    {
        uint32_t rasr;
        uint64_t base;
        uint64_t end;
        uint32_t sizeField;

        MPU->RNR = (uint32_t)r;
        rasr = MPU->RASR;
        if ((rasr & MPU_RASR_ENABLE_Msk) == 0U)
        {
            continue;
        }

        sizeField = (rasr & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos;
        base = (uint64_t)(MPU->RBAR & MPU_RBAR_ADDR_Msk);
        end = base + (1ULL << (sizeField + 1U));

        if ((end <= HAL_NOCACHE_BASE) || (base >= HAL_NOCACHE_END))
        {
            continue; /* does not touch the window */
        }

        if ((base > HAL_NOCACHE_BASE) || (end < HAL_NOCACHE_END))
        {
            return false; /* only partial coverage */
        }

        return ((((rasr & MPU_RASR_TEX_Msk) >> MPU_RASR_TEX_Pos)
                    == HAL_NOCACHE_TEX_NORMAL_NC)
                && ((rasr & MPU_RASR_C_Msk) == 0U)
                && ((rasr & MPU_RASR_B_Msk) == 0U)
                && ((rasr & MPU_RASR_SRD_Msk) == 0U));
    }

    return false;
}
