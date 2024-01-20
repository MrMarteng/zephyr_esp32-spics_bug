/**
 * @file main.cpp
 * @brief
 * @author Martin Kiepfer (mrmarteng@teleschirm.org)
 * @attention
 */
#include "zephyr/drivers/spi.h"
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(SPITest, CONFIG_APP_LOG_LEVEL);

K_THREAD_STACK_DEFINE(spiSlowThreadStack, 1024u);
static struct k_thread slowDataThreadData;

static void spiSlowDataTransfer(void*, void*, void*);

/**********************
 *   APPLICATION MAIN
 **********************/
int main(void)
{
    const struct device* pSpi;

    pSpi = DEVICE_DT_GET(DT_NODELABEL(spi2));

    if (pSpi != NULL)
    {
        struct spi_config config;
        struct spi_buf_set spi_bufset;
        struct spi_buf spi_buf;
        static uint8_t data[1024u] = {};

        memset(&config.cs, 0, sizeof(config.cs));
        config.cs.gpio.port = DEVICE_DT_GET(DT_NODELABEL(gpio0));
        config.cs.gpio.pin = 1;
        config.cs.gpio.dt_flags = GPIO_ACTIVE_LOW;
        config.frequency = 500000u;
        config.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_MODE_CPHA | SPI_MODE_CPOL;
        gpio_pin_configure(config.cs.gpio.port, config.cs.gpio.pin, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_LOW);

        k_tid_t slowDataTid = k_thread_create(&slowDataThreadData, spiSlowThreadStack, K_THREAD_STACK_SIZEOF(spiSlowThreadStack),
                                              spiSlowDataTransfer, NULL, NULL, NULL, 50, 0, K_NO_WAIT);

        memset(data, 0xFFu, sizeof(data));
        spi_buf.buf = data;
        spi_buf.len = sizeof(data);
        spi_bufset.buffers = &spi_buf;
        spi_bufset.count = 1;

        gpio_pin_configure(config.cs.gpio.port, config.cs.gpio.pin, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_LOW);

        while (true)
        {
            spi_write(pSpi, &config, &spi_bufset);
            k_sleep(K_MSEC(1000));
        }
    }

    return 0;
}

static void spiSlowDataTransfer(void*, void*, void*)
{

    const struct device* pSpi = DEVICE_DT_GET(DT_NODELABEL(spi2));
    struct spi_config config;
    struct spi_buf_set spi_bufset;
    struct spi_buf spi_buf;
    static uint8_t data[512u] = {0x00};

    memset(data, 0x00u, sizeof(data));

    memset(&config.cs, 0, sizeof(config.cs));
    config.cs.gpio.port = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    config.cs.gpio.pin = 10;
    config.cs.gpio.dt_flags = GPIO_ACTIVE_LOW;
    config.frequency = 250000u;
    config.operation =
        SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_MODE_CPHA | /*SPI_LOCK_ON | SPI_HOLD_ON_CS | */ SPI_MODE_CPOL;

    gpio_pin_configure(config.cs.gpio.port, config.cs.gpio.pin, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_LOW);

    k_sleep(K_MSEC(3000));

    while (true)
    {
        spi_buf.buf = data;
        spi_buf.len = sizeof(data);
        spi_bufset.buffers = &spi_buf;
        spi_bufset.count = 1;

        spi_write(pSpi, &config, &spi_bufset);

        k_sleep(K_MSEC(300));
    }
}
