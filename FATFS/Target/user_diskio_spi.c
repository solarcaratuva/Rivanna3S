/**
 * @file user_diskio_spi.c
 * @brief SPI-based SD card disk I/O driver for FatFS.
 *
 * Based on ChaN's SD card driver for FatFS.
 * Adapted for STM32 HAL.
 */

#include "sd_spi_config.h"
#include "user_diskio_spi.h"
#include "stm32h743xx.h"
#include "stm32h7xx_hal.h"
#include "diskio.h"
#include "ff.h"

/* SPI clock speed control macros */
/* For STM32H7, adjust prescaler based on your SPI clock source */
/* These provide approximately 400kHz for init and higher speed for normal operation */
#define FCLK_SLOW()                                                                        \
    {                                                                                      \
        MODIFY_REG(SD_SPI_HANDLE.Instance->CFG1, SPI_CFG1_MBR, SPI_BAUDRATEPRESCALER_256); \
    }
#define FCLK_FAST()                                                                      \
    {                                                                                    \
        MODIFY_REG(SD_SPI_HANDLE.Instance->CFG1, SPI_CFG1_MBR, SPI_BAUDRATEPRESCALER_8); \
    }

/* CS pin control */
#define CS_HIGH() HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)
#define CS_LOW() HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)

/* Definitions for MMC/SDC commands */
#define CMD0 (0)           /* GO_IDLE_STATE */
#define CMD1 (1)           /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (8)           /* SEND_IF_COND */
#define CMD9 (9)           /* SEND_CSD */
#define CMD10 (10)         /* SEND_CID */
#define CMD12 (12)         /* STOP_TRANSMISSION */
#define CMD13 (13)         /* SEND_STATUS */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16 (16)         /* SET_BLOCKLEN */
#define CMD17 (17)         /* READ_SINGLE_BLOCK */
#define CMD18 (18)         /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)         /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)         /* WRITE_BLOCK */
#define CMD25 (25)         /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)         /* ERASE_WR_BLK_START */
#define CMD33 (33)         /* ERASE_WR_BLK_END */
#define CMD38 (38)         /* ERASE */
#define CMD55 (55)         /* APP_CMD */
#define CMD58 (58)         /* READ_OCR */

/* Card type flags */
#define CT_MMC 0x01              /* MMC ver 3 */
#define CT_SD1 0x02              /* SD ver 1 */
#define CT_SD2 0x04              /* SD ver 2 */
#define CT_SDC (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK 0x08            /* Block addressing */

/* Static variables */
static volatile DSTATUS Stat = STA_NOINIT; /* Disk status */
static BYTE CardType;                      /* Card type flags */

/* Timer functions - use HAL tick */
static inline DWORD get_fattime_ticks(void)
{
    return HAL_GetTick();
}

/* SPI send/receive byte */
static BYTE spi_xchg(BYTE dat)
{
    BYTE rxDat;
    HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &dat, &rxDat, 1, HAL_MAX_DELAY);
    return rxDat;
}

/* SPI receive multiple bytes */
static void spi_rcvr_multi(BYTE *buff, UINT cnt)
{
    for (UINT i = 0; i < cnt; i++)
    {
        buff[i] = spi_xchg(0xFF);
    }
}

/* SPI transmit multiple bytes */
static void spi_xmit_multi(const BYTE *buff, UINT cnt)
{
    for (UINT i = 0; i < cnt; i++)
    {
        spi_xchg(buff[i]);
    }
}

/* Wait for card ready */
static int wait_ready(UINT wt)
{
    BYTE d;
    DWORD start = get_fattime_ticks();

    do
    {
        d = spi_xchg(0xFF);
    } while (d != 0xFF && (get_fattime_ticks() - start) < wt);

    return (d == 0xFF) ? 1 : 0;
}

/* Deselect card and release SPI */
static void deselect(void)
{
    CS_HIGH();
    spi_xchg(0xFF); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/* Select card and wait for ready */
static int select(void)
{
    CS_LOW();
    spi_xchg(0xFF); /* Dummy clock (force DO enabled) */

    if (wait_ready(500))
        return 1; /* Wait for card ready */

    deselect();
    return 0; /* Timeout */
}

/* Receive a data packet from the card */
static int rcvr_datablock(BYTE *buff, UINT btr)
{
    BYTE token;
    DWORD start = get_fattime_ticks();

    /* Wait for data packet */
    do
    {
        token = spi_xchg(0xFF);
    } while ((token == 0xFF) && ((get_fattime_ticks() - start) < 200));

    if (token != 0xFE)
        return 0; /* Invalid token */

    /* Receive the data block */
    spi_rcvr_multi(buff, btr);

    /* Discard CRC */
    spi_xchg(0xFF);
    spi_xchg(0xFF);

    return 1;
}

/* Send a data packet to the card */
static int xmit_datablock(const BYTE *buff, BYTE token)
{
    BYTE resp;

    if (!wait_ready(500))
        return 0;

    /* Send token */
    spi_xchg(token);

    if (token != 0xFD)
    { /* Not StopTran token */
        /* Send data */
        spi_xmit_multi(buff, 512);

        /* Send dummy CRC */
        spi_xchg(0xFF);
        spi_xchg(0xFF);

        /* Receive response token */
        resp = spi_xchg(0xFF);

        /* Check if data accepted */
        if ((resp & 0x1F) != 0x05)
            return 0;
    }

    return 1;
}

/* Send a command packet to the card */
static BYTE send_cmd(BYTE cmd, DWORD arg)
{
    BYTE n, res;

    /* Send CMD55 for ACMD */
    if (cmd & 0x80)
    {
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1)
            return res;
    }

    /* Select card */
    if (cmd != CMD12)
    {
        deselect();
        if (!select())
            return 0xFF;
    }

    /* Send command packet */
    spi_xchg(0x40 | cmd);        /* Start + command index */
    spi_xchg((BYTE)(arg >> 24)); /* Argument[31..24] */
    spi_xchg((BYTE)(arg >> 16)); /* Argument[23..16] */
    spi_xchg((BYTE)(arg >> 8));  /* Argument[15..8] */
    spi_xchg((BYTE)arg);         /* Argument[7..0] */

    /* Send CRC */
    n = 0x01; /* Dummy CRC + Stop */
    if (cmd == CMD0)
        n = 0x95; /* Valid CRC for CMD0 */
    if (cmd == CMD8)
        n = 0x87; /* Valid CRC for CMD8 */
    spi_xchg(n);

    /* Receive response */
    if (cmd == CMD12)
        spi_xchg(0xFF); /* Skip a stuff byte on CMD12 */

    n = 10; /* Wait for response (max 10 bytes) */
    do
    {
        res = spi_xchg(0xFF);
    } while ((res & 0x80) && --n);

    return res;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS USER_SPI_initialize(BYTE pdrv)
{
    BYTE n, cmd, ty, ocr[4];
    DWORD start;

    if (pdrv != 0)
        return STA_NOINIT; /* Only drive 0 supported */

    /* Initialize SPI at slow speed for card detection */
    FCLK_SLOW();

    /* Send 80 dummy clocks with CS high */
    CS_HIGH();
    for (n = 10; n; n--)
        spi_xchg(0xFF);

    ty = 0;
    if (send_cmd(CMD0, 0) == 1)
    { /* Enter Idle state */
        start = get_fattime_ticks();

        if (send_cmd(CMD8, 0x1AA) == 1)
        { /* SDv2? */
            /* Get 32-bit return value of R7 response */
            for (n = 0; n < 4; n++)
                ocr[n] = spi_xchg(0xFF);

            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            { /* Can work at VDD range of 2.7-3.6V */
                /* Wait for leaving idle state (ACMD41 with HCS bit) */
                while ((get_fattime_ticks() - start) < 1000 && send_cmd(ACMD41, 1UL << 30))
                    ;

                /* Check CCS bit in the OCR */
                if ((get_fattime_ticks() - start) < 1000 && send_cmd(CMD58, 0) == 0)
                {
                    for (n = 0; n < 4; n++)
                        ocr[n] = spi_xchg(0xFF);
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* SDv2 */
                }
            }
        }
        else
        { /* SDv1 or MMCv3 */
            if (send_cmd(ACMD41, 0) <= 1)
            {
                ty = CT_SD1;
                cmd = ACMD41; /* SDv1 */
            }
            else
            {
                ty = CT_MMC;
                cmd = CMD1; /* MMCv3 */
            }
            /* Wait for leaving idle state */
            while ((get_fattime_ticks() - start) < 1000 && send_cmd(cmd, 0))
                ;

            /* Set block length to 512 */
            if ((get_fattime_ticks() - start) >= 1000 || send_cmd(CMD16, 512) != 0)
            {
                ty = 0;
            }
        }
    }

    CardType = ty;
    deselect();

    if (ty)
    {
        /* Initialization successful */
        FCLK_FAST();
        Stat &= ~STA_NOINIT;
    }
    else
    {
        /* Initialization failed */
        Stat = STA_NOINIT;
    }

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS USER_SPI_status(BYTE pdrv)
{
    if (pdrv != 0)
        return STA_NOINIT;
    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT USER_SPI_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    BYTE cmd;

    if (pdrv != 0 || count == 0)
        return RES_PARERR;
    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    /* Convert sector to byte address if needed */
    if (!(CardType & CT_BLOCK))
        sector *= 512;

    cmd = (count > 1) ? CMD18 : CMD17; /* READ_MULTIPLE_BLOCK or READ_SINGLE_BLOCK */

    if (send_cmd(cmd, sector) == 0)
    {
        do
        {
            if (!rcvr_datablock(buff, 512))
                break;
            buff += 512;
        } while (--count);

        if (cmd == CMD18)
            send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT USER_SPI_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || count == 0)
        return RES_PARERR;
    if (Stat & STA_NOINIT)
        return RES_NOTRDY;
    if (Stat & STA_PROTECT)
        return RES_WRPRT;

    /* Convert sector to byte address if needed */
    if (!(CardType & CT_BLOCK))
        sector *= 512;

    if (count == 1)
    {
        /* Single block write */
        if ((send_cmd(CMD24, sector) == 0) && xmit_datablock(buff, 0xFE))
        {
            count = 0;
        }
    }
    else
    {
        /* Multiple block write */
        if (CardType & CT_SDC)
            send_cmd(ACMD23, count);

        if (send_cmd(CMD25, sector) == 0)
        {
            do
            {
                if (!xmit_datablock(buff, 0xFC))
                    break;
                buff += 512;
            } while (--count);

            if (!xmit_datablock(0, 0xFD))
                count = 1; /* STOP_TRAN token */
        }
    }
    deselect();

    return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT USER_SPI_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res;
    BYTE n, csd[16];
    DWORD cs;

    if (pdrv != 0)
        return RES_PARERR;

    res = RES_ERROR;

    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    switch (cmd)
    {
    case CTRL_SYNC:
        /* Make sure all pending write operations are finished */
        if (select())
        {
            deselect();
            res = RES_OK;
        }
        break;

    case GET_SECTOR_COUNT:
        /* Get number of sectors on the disk */
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
        {
            if ((csd[0] >> 6) == 1)
            {
                /* SDC ver 2.00 */
                cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
                *(DWORD *)buff = cs << 10;
            }
            else
            {
                /* SDC ver 1.XX or MMC */
                n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                *(DWORD *)buff = cs << (n - 9);
            }
            res = RES_OK;
        }
        deselect();
        break;

    case GET_BLOCK_SIZE:
        /* Get erase block size in unit of sector */
        *(DWORD *)buff = 128; /* Default value */
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    return res;
}
