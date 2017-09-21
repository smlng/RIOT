/*
 * Copyright (C)    2017 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup      cpu_sam4s2b
 * @{
 *
 * @file
 * @brief           Memory definitions for the SAM3X8E
 *
 * @author          Sebastian Meiling <s@mlng.net>
 *
 * @}
 */

MEMORY
{
    rom (rx)    : ORIGIN = 0x00400000, LENGTH = 128K
    ram (rwx)   : ORIGIN = 0x20000000, LENGTH = 64K
}

INCLUDE cortexm_base.ld
