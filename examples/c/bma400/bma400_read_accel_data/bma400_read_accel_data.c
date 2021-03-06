/**
 * Copyright (C) 2018 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @file    bma400_read_accel_data.c
 * @brief   Sample code to read accel. data from BMA400 using COINES library
 *
 */

/*********************************************************************/
/* system header files */
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*********************************************************************/
/* own header files */
/*********************************************************************/
#include "coines.h"
#include "bma400.h"
/*********************************************************************/
/* macro definitions */
/*********************************************************************/
/*! i2c interface communication, 1 - Enable; 0- Disable */
#define BMA400_INTERFACE_I2C             1
/*! spi interface communication, 1 - Enable; 0- Disable */
#define BMA400_INTERFACE_SPI             0

#if (!((BMA400_INTERFACE_I2C==1) && (BMA400_INTERFACE_SPI==0)) && \
	(!((BMA400_INTERFACE_I2C==0) && (BMA400_INTERFACE_SPI==1))))
#error "Invalid value given for the macros BMA400_INTERFACE_I2C / BMA400_INTERFACE_SPI"
#endif
/*! BMA400 shuttle board ID */
#define BMA400_SHUTTLE_ID           0x1A1
/*! This variable holds the device address of BMA400 */
#define BMA400_DEV_ADDR BMA400_I2C_ADDR_PRIM

#define GRAVITY_EARTH (9.80665f) /* Earth's gravity in m/s^2 */

/*********************************************************************/
/* global variables */

/*********************************************************************/
/* function declarations */
/*********************************************************************/

/*!
 * @brief	This internal API is used to initialize the sensor interface
 *
 * @param	None
 * @return	None
 */
static void init_sensor_interface(void);

/*!
 * @brief	This internal API is used to initialize the bma400 sensor with default settings
 *
 * @param	Pointer to bma400 device structure
 * @return	None
 */
static void init_bma400(struct bma400_dev *bma400dev);

/*!
 * @brief	 This internal API is used to set the sensor driver interface to read/write the data
 *
 * @param	Pointer to bma400 device structure
 * @return	None
 */
static void init_bma400_sensor_driver_interface(struct bma400_dev *bma400dev);

/*!
 *  @brief Delay in milliseconds.
 *
 *  @param Period
 */
void delay_ms(uint32_t period);

/*!
 * @brief Function to to print the result.
 *
 * @param rslt
 * @return None
 */
void print_rslt(int8_t rslt);
/*!  @brief This internal API is used to convert lsb to ms2
 *
 *  @param[in] val: value
 *  @param[in] range: range value
 *  @param[in] bit_width
 *
 *  @return float value in ms2
 */
float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width);
/*!
 *  @brief This internal API is used to convert ticks to seconds
 *
 *  @param[in] sensor_time

 *
 *  @return float value in seconds
 *
 */
float sensor_ticks_to_s(uint32_t sensor_time);

/*!
 *  @brief This internal API is used to initialize the sensor interface depending
 *   on selection either SPI or I2C.
 *
 *  @param[in] interface
 *
 *  @return void
 *
 */
static void init_sensor_interface(void)
{
    /* Switch VDD for sensor off */
    coines_set_shuttleboard_vdd_vddio_config(0, 0);
    /* wait until the sensor goes off */
    coines_delay_msec(10);
#if BMA400_INTERFACE_I2C==1       
    /* set the sensor interface as I2C */
    coines_config_i2c_bus(COINES_I2C_BUS_0, COINES_I2C_FAST_MODE);

#elif BMA400_INTERFACE_SPI==1
    /* set the sensor interface as SPI */
    coines_config_spi_bus(COINES_SPI_BUS_0, COINES_SPI_SPEED_5_MHZ, COINES_SPI_MODE3);

#endif
    /* Switch VDD for sensor on */
    coines_set_shuttleboard_vdd_vddio_config(3300, 3300);
}

/*!
 * @brief This internal API is used to initializes the bma400 sensor with default
 * settings like power mode and OSRS settings
 *
 * @param[in] void
 *
 * @return void
 *
 */
static void init_bma400(struct bma400_dev *bma400dev)
{
    int8_t rslt;
    rslt = bma400_init(bma400dev);
    if (rslt == BMA400_OK)
    {
        printf("BMA400 Initialization Success!\n");
        printf("Chip ID 0x%x\n", bma400dev->chip_id);
    }
    else
    {
        print_rslt(rslt);
    }
    coines_delay_msec(100);
}
/*!
 *  @brief This internal API is used to print the result.
 */
void print_rslt(int8_t rslt)
{
    switch (rslt)
    {
        case BMA400_OK:
            /* Do nothing */
            break;
        case BMA400_E_NULL_PTR:
            printf("Error [%d] : Null pointer\r\n", rslt);
            break;
        case BMA400_E_COM_FAIL:
            printf("Error [%d] : Communication failure\r\n", rslt);
            break;
        case BMA400_E_DEV_NOT_FOUND:
            printf("Error [%d] : Device not found\r\n", rslt);
            break;
        case BMA400_E_INVALID_CONFIG:
            printf("Error [%d] : Invalid configuration\r\n", rslt);
            break;
        case BMA400_W_SELF_TEST_FAIL:
            printf("Warning [%d] : Self test failed\r\n", rslt);
            break;
        default:
            printf("Error [%d] : Unknown error code\r\n", rslt);
            break;
    }
    fflush(stdout);

    if (rslt != BMA400_OK)
    {
        exit(COINES_E_FAILURE);
    }
}

/*!
 *  @brief This internal API is used to set the sensor driver interface to
 *  read/write the data.
 *
 *  @param[in] bma400dev: bma400 device structure
 *
 *  @return void
 *
 */
static void init_bma400_sensor_driver_interface(struct bma400_dev *bma400dev)
{
#if BMA400_INTERFACE_I2C==1
    /* I2C setup */
    /* link read/write/delay function of host system to appropriate
     * bma400 function call prototypes */
    bma400dev->intf_ptr = NULL; /* To attach your interface device reference */
    bma400dev->write = coines_write_i2c;
    bma400dev->read = coines_read_i2c;
    bma400dev->delay_ms = coines_delay_msec;
    /* set correct i2c address */
    bma400dev->dev_id = (unsigned char) BMA400_I2C_ADDRESS_SDO_LOW;
    bma400dev->intf = BMA400_I2C_INTF;

#elif BMA400_INTERFACE_SPI==1
    /* SPI setup */
    /* link read/write/delay function of host system to appropriate
     *  bma400 function call prototypes */
    bma400dev->intf_ptr = NULL; /* To attach your interface device reference */
    bma400dev->write = coines_write_spi;
    bma400dev->read = coines_read_spi;
    bma400dev->delay_ms = coines_delay_msec;
    bma400dev->intf = BMA400_SPI_INTF;
    bma400dev->dev_id = COINES_SHUTTLE_PIN_7;
#endif
}

/*!
 *  @brief This internal API is used to configure the sensor 
 *
 *  @param[in] bma400dev: bma400 device structure
 *
 *  @return Results of API execution status.
 *  @retval 0 -> Success
 *  @retval Any non zero value -> Fail
 *
 */
static int8_t set_sensor_config(struct bma400_dev *bma400dev)
{
    int8_t rslt;
    struct bma400_sensor_conf conf;

    /* Select the type of configuration to be modified */
    conf.type = BMA400_ACCEL;

    /* Get the accelerometer configurations which are set in the sensor */
    rslt = bma400_get_sensor_conf(&conf, 1, bma400dev);
    print_rslt(rslt);

    /* Modify the desired configurations as per macros
     * available in bma400_defs.h file */
    conf.param.accel.odr = BMA400_ODR_100HZ;
    conf.param.accel.range = BMA400_2G_RANGE;
    conf.param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

    /* Set the desired configurations to the sensor */
    rslt = bma400_set_sensor_conf(&conf, 1, bma400dev);
    print_rslt(rslt);

    rslt = bma400_set_power_mode(BMA400_LOW_POWER_MODE, bma400dev);
    print_rslt(rslt);

    return rslt;
}

/*!
 *  @brief This internal API is used to convert lsb to ms2 
 *
 *  @param[in] val: value
 *  @param[in] range: range value
 *  @param[in] bit_width
 *
 *  @return float value in ms2
 *
 */
float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width)
{
    float half_scale = (float)(1 << bit_width) / 2.0f;

    return GRAVITY_EARTH * val * g_range / half_scale;
}
/*!
 *  @brief This internal API is used to convert ticks to seconds 
 *
 *  @param[in] sensor_time

 *
 *  @return float value in seconds
 *
 */
float sensor_ticks_to_s(uint32_t sensor_time)
{
    return (float)sensor_time * 0.0000390625f;
}

/*!
 *  @brief Main Function where the execution getting started to test the code.
 *
 *  @param[in] argc
 *  @param[in] argv
 *
 *  @return status
 *
 */
int main(int argc, char const *argv[])
{
    struct coines_board_info board_info;
    struct bma400_sensor_data data;
    struct bma400_dev bma;
    int8_t rslt;
    uint8_t n_samples = 200;
    float t, x, y, z;
    init_bma400_sensor_driver_interface(&bma);

    rslt = coines_open_comm_intf(COINES_COMM_INTF_USB);
    if (rslt < 0)
    {
        printf("\n Unable to connect with Application Board ! \n"
               " 1. Check if the board is connected and powered on. \n"
               " 2. Check if Application Board USB driver is installed. \n"
               " 3. Check if board is in use by another application. (Insufficient permissions to access USB) \n");
        exit(rslt);
    }

    /* Check if the right board is connected (implicitly check if board was reset) */
    rslt = coines_get_board_info(&board_info);
    if (rslt == COINES_SUCCESS)
    {
        if (board_info.shuttle_id != BMA400_SHUTTLE_ID)
        {
            printf("Invalid sensor shuttle ID (not a BMA400 shuttle)\n(sometimes board power off-power on helps.)\n");
            fflush(stdout);
            exit(COINES_E_FAILURE);
        }
    }

    init_sensor_interface();

    /* after sensor init introduce 200 msec sleep */
    coines_delay_msec(200);

    init_bma400(&bma);

    rslt = bma400_soft_reset(&bma);
    print_rslt(rslt);

    rslt = set_sensor_config(&bma);
    if (rslt != BMA400_OK)
    {
        printf("Error setting bma400 config.\n");
        fflush(stdout);
        exit(rslt);
    }

    while (n_samples && (rslt == BMA400_OK))
    {
        bma.delay_ms(10); /* Wait for 10ms as ODR is set to 100Hz */

        rslt = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &data, &bma);
        print_rslt(rslt);

        /* 12-bit accelerometer at range 2G */
        x = lsb_to_ms2(data.x, 2, 12);
        y = lsb_to_ms2(data.y, 2, 12);
        z = lsb_to_ms2(data.z, 2, 12);
        t = sensor_ticks_to_s(data.sensortime);

        printf("t[s]:%.4f\tdata[m/s2]: ax:%.4f\tay:%.4f\taz:%.4f\n", t, x, y,
               z);
        fflush(stdout);

        n_samples--;
    }

    return 0;
}
