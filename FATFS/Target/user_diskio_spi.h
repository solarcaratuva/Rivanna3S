// Based on tutorial from https://01001000.xyz/2020-08-09-Tutorial-STM32CubeIDE-SD-card/

/**
 * @file user_diskio_spi.h
 * @brief SPI-based SD card disk I/O driver for FatFS.
 */

#ifndef USER_DISKIO_SPI_H
#define USER_DISKIO_SPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ff_gen_drv.h"

    /* Prototypes for disk I/O functions */
    DSTATUS USER_SPI_initialize(BYTE pdrv);
    DSTATUS USER_SPI_status(BYTE pdrv);
    DRESULT USER_SPI_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
    DRESULT USER_SPI_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
    DRESULT USER_SPI_ioctl(BYTE pdrv, BYTE cmd, void *buff);

#ifdef __cplusplus
}
#endif

#endif /* USER_DISKIO_SPI_H */
