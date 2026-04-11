/**
 * @file SD.h
 * @brief SD card telemetry logging wrapper
 *
 * This header defines the SdCard class, a small C++ wrapper around the
 * FatFs and SPI layers used to mount the SD card, append telemetry log data,
 * and periodically flush buffered writes to storage.
 *
 * The class owns the SD card state, a background flush thread, and an
 * internal message queue used to decouple logging from file I/O.
 *
 * @note Log data is only accepted once the card has been successfully mounted
 * and the log file has been opened.
 */
#ifndef SD_H
#define SD_H

#include "fatfs.h"
#include "thread.h"
#include "MessageQueue.h"

class SPI;

/**
 * @brief Interval between background flush attempts, in milliseconds
 */
#define SD_FLUSH_INTERVAL_MS 1000

/**
 * @class SdCard
 * @brief High-level wrapper for writing telemetry logs to an SD card
 *
 * This class initializes the SD card over SPI, mounts the FatFs volume,
 * opens the telemetry log file in append mode, and forwards log data to a
 * background flush thread through an internal message queue.
 *
 * Data submitted through write() is buffered when the card is ready, and the
 * flush loop periodically drains that buffer and persists it to storage.
 */
class SdCard {
    private:
        SPI* _spi;
        uint8_t _retUSER;
        char _USERPath[4];
        FATFS _USERFatFS;
        FIL _USERFile;
        Thread _flush_thread;
        bool _ready;
        MessageQueue _mq;
        static SdCard* _SdCard;

        /**
         * @brief Background worker that drains the write queue and flushes the file
         */
        static void continous_flush();

    public:
        /**
         * @brief Construct an SdCard instance and initialize the SD logging stack
         * @param spi_peripheral SPI peripheral used to communicate with the SD card
         *
         * The constructor links the FatFs driver, mounts the volume, opens the
         * telemetry log file in append mode, and starts the background flush thread
         * if initialization succeeds.
         */
        SdCard(SPI* spi_peripheral);

        /**
         * @brief Queue telemetry data for writing to the SD card
         * @param buffer Pointer to the data to enqueue
         * @param len Number of bytes to enqueue
         * @return Number of bytes accepted, or 0 if the card is not ready or the queue is full
         */
        int write(uint8_t* buffer, uint16_t len);

        /**
         * @brief Drain queued telemetry data and flush it to the SD card
         *
         * This method blocks on the internal message queue, writes the received
         * payload to the open log file, synchronizes the file system state, and
         * waits before the next flush cycle.
         */
        void write_and_flush_loop();

        /**
         * @brief Register the SD card as the logging output destination
         *
         * After this call, log messages produced by the logging subsystem are
         * forwarded into the SD card write queue when the card is ready.
         */
        void attach_to_log();
};

        #endif // SD_H
