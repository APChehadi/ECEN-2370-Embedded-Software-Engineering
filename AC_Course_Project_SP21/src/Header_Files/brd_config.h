#ifndef BRD_CONFIG_HG
#define BRD_CONFIG_HG

//***********************************************************************************
// Include files
//***********************************************************************************
/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_gpio.h"

/* The developer's include statements */


//***********************************************************************************
// defined files
//***********************************************************************************

// GPIO pin setup
#define STRONG_DRIVE

// LED 0 pin is
#define	LED0_PORT				gpioPortF
#define LED0_PIN				04u
#define LED0_DEFAULT			false 	// Default false (0) = off, true (1) = on
#define LED0_GPIOMODE			gpioModePushPull

// LED 1 pin is
#define LED1_PORT				gpioPortF
#define LED1_PIN				05u
#define LED1_DEFAULT			false	// Default false (0) = off, true (1) = on
#define LED1_GPIOMODE			gpioModePushPull

#ifdef STRONG_DRIVE
	#define LED0_DRIVE_STRENGTH		gpioDriveStrengthStrongAlternateStrong
	#define LED1_DRIVE_STRENGTH		gpioDriveStrengthStrongAlternateStrong
#else
	#define LED0_DRIVE_STRENGTH		gpioDriveStrengthWeakAlternateWeak
	#define LED1_DRIVE_STRENGTH		gpioDriveStrengthWeakAlternateWeak
#endif


// System Clock setup
#define MCU_HFXO_FREQ			cmuHFRCOFreq_19M0Hz


// LETIMER PWM Configuration
#define		PWM_ROUTE_0			LETIMER_ROUTELOC0_OUT0LOC_LOC28
#define		PWM_ROUTE_1			LETIMER_ROUTELOC0_OUT1LOC_LOC28


// Si7021 Dependencies
#define 	SI7021_SCL_PORT 		gpioPortC
#define 	SI7021_SCL_PIN 			11u
#define		SI7021_SCL_ROUTE		I2C_ROUTELOC0_SCLLOC_LOC19

#define 	SI7021_SDA_PORT 		gpioPortC
#define 	SI7021_SDA_PIN 			10u
#define		SI7021_SDA_ROUTE		I2C_ROUTELOC0_SDALOC_LOC19

#define 	SI7021_SENSOR_EN_PORT 	gpioPortB
#define 	SI7021_SENSOR_EN_PIN 	10u

#define 	SI7021_DRIVE_STRENGTH	gpioDriveStrengthWeakAlternateWeak
#define 	SI7021_SENSOR_GPIOMDOE	gpioModePushPull
#define 	SI7021_SENSOR_DEFAULT	true
#define 	SI7021_I2C_GPIOMODE		gpioModeWiredAnd
#define 	SI7021_I2C_DEFAULT		true


// VEML Dependencies
#define 	VEML_SCL_PORT 			gpioPortB
#define 	VEML_SCL_PIN 			12u
#define		VEML_SCL_ROUTE			I2C_ROUTELOC0_SCLLOC_LOC6

#define 	VEML_SDA_PORT 			gpioPortB
#define 	VEML_SDA_PIN 			13u
#define		VEML_SDA_ROUTE			I2C_ROUTELOC0_SDALOC_LOC8

#define 	VEML_DRIVE_STRENGTH		gpioDriveStrengthWeakAlternateWeak
#define 	VEML_SENSOR_GPIOMODE	gpioModeWiredAnd
#define 	VEML_I2C_DEFAULT		true


// CC2640R2 Bluetooth Module Dependencies
#define		LEUART0_TX_PORT			gpioPortD
#define		LEUART0_TX_PIN			10u
#define		LEUART0_TX_DEFAULT		true
#define		LEUART0_TX_GPIOMODE		gpioModePushPull

#define		LEUART0_RX_PORT			gpioPortD
#define		LEUART0_RX_PIN			11u
#define		LEUART0_RX_DEFAULT		true
#define		LEUART0_RX_GPIOMODE		gpioModeInput

#define 	LEUART0_DRIVE_STRENGTH	gpioDriveStrengthStrongAlternateWeak

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************

#endif
