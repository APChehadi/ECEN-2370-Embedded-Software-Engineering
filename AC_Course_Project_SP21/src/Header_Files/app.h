/*
 * i2c.h
 *
 *  Created on: Feb 07, 2021
 *      Author: adamp
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	APP_HG
#define	APP_HG

// Application scheduled events
#define LETIMER0_COMP0_CB 		0x00000001 		//0b00001
#define LETIMER0_COMP1_CB 		0x00000002 		//0b00010
#define LETIMER0_UF_CB 			0x00000004 		//0b00100
#define	SI7021_READ_CB			0x00000008  	//0b01000
#define	BOOT_UP_CB				0x10			//0b10000
#define BLE_TX_DONE_CB			0x20			//0b
#define BLE_RX_DONE_CB			0x40
#define VEML_CB					0x80
#define SI7021_TEMP_READ_CB 	0x100

/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "sleep_routines.h"
#include "i2c.h"
#include "SI7021.h"
#include "ble.h"
#include "HW_delay.h"
#include "stdio.h"
#include "veml.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define		PWM_PER				1.8		// PWM period in seconds
#define		PWM_ACT_PER			0.25	// PWM active period in seconds

#define		DELAY				2000	// scheduled_boot_up_cb timer delay
#define		SYSTEM_BLOCK_EM		EM3

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);

void scheduled_letimer0_comp0_cb(void);
void scheduled_letimer0_comp1_cb(void);
void scheduled_letimer0_uf_cb(void);
void scheduled_si7021_humidity_cb(void);
void scheduled_si7021_temp_cb(void);
void scheduled_veml_read_cb(void);
void scheduled_boot_up_cb(void);
void scheduled_ble_tx_done_cb(void);

#endif
