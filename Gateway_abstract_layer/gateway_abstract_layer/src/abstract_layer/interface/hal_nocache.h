#ifndef HAL_NOCACHE_H
#define HAL_NOCACHE_H

/*
 * Non-cacheable SRAM window for structures shared with peripheral DMA
 * (MCAN Message RAM, GMAC buffer descriptors). The Cortex-M7 D-cache must not
 * hold these lines: the MCC plib (MCAN) and the GMAC DMA access them without
 * cache maintenance.
 *
 * Covered by MCC -> System -> MPU, Region 5 (Memory Space left empty):
 *   Start Address 0x2045F800, Region Size 2 KB,
 *   Memory Type "Normal memory, Non-cacheable",
 *   Data Access Read/Write, Instruction Fetch off (= execute never).
 * Keep the region exactly this size: the linker places the main stack right
 * below the pinned data (_stack = 0x2045F7F8), so a larger region (e.g. 4 KB
 * at 0x2045F000) would make the top of the stack non-cacheable and slow.
 * hal_nocache_region_ok() verifies the MPU at start-up.
 *
 * Layout (2 KB):
 *   0x2045F800  MCAN1 Message RAM  (MCAN1_MESSAGE_RAM_CONFIG_SIZE = 1816 B)
 *   0x2045FF20  GMAC descriptors   (hal_eth.c, <= 224 B)
 */

#define HAL_NOCACHE_BASE        (0x2045F800UL)
#define HAL_NOCACHE_SIZE        (0x800UL)
#define HAL_NOCACHE_END         (HAL_NOCACHE_BASE + HAL_NOCACHE_SIZE)

#define HAL_NOCACHE_MCAN_ADDR   (0x2045F800UL)
#define HAL_NOCACHE_MCAN_MAX    (0x720UL)

#define HAL_NOCACHE_GMAC_ADDR   (0x2045FF20UL)
#define HAL_NOCACHE_GMAC_MAX    (HAL_NOCACHE_END - HAL_NOCACHE_GMAC_ADDR)

#endif /* HAL_NOCACHE_H */
