#####################################################################################################
This example reads the chip-id of bmp38x sensor 

Uses bmp3_init() sensor API 
Chip ID can be read from 'bmp3_dev' structure member 'chip_id' 

#####################################################################################################
Chip id read 
1. Set the sensor interface SPI/I2C using bmp38x sensor api  and map coines read/write interface calls to sensor api calls.
/* macro definitions */
/*enables i2c interface communication*/
#define BMP3_INTERFACE_I2C             1
/*enables spi interface communication*/
#define BMP3_INTERFACE_SPI             0

2. Initialize the application/board communication interface (USB) using coines API
3. Read the board information and check the valid shuttle id 
4. Initialize the sensor interface 
5. Wait for 200ms 
6. Initialize the sensors (sensor init function takes a configuration structure where several configuration can be specified).
7. Close the communication interface
#####################################################################################################
