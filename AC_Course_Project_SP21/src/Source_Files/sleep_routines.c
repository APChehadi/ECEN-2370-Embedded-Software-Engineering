/*
 * sleep_routines.c
 *
 *  Created on: Feb 16, 2021
 *      Author: adamp
 */

/**************************************************************************
* @file sleep_routines.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Initialize sleep routine
 *
 * @details
 *	This function call will initialize the static array lowest_energy_mode.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void sleep_open(void) {
	for (int i = 0; i < MAX_ENERGY_MODES; i++) {
		lowest_energy_mode[i] = 0;
	}
}


/***************************************************************************//**
 * @brief
 *   Prevent going into sleep mode while active
 *
 * @details
 * 	 This function call will prevent the Pearl Gecko going into sleep mode while the peripheral is active.
 *
 * @note
 *   This function does not return any values.
 *
 * @param[in] EM
 *   Energy Mode to increment
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]++;
	EFM_ASSERT(lowest_energy_mode[EM] <= 5);

	CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
 * @brief
 *   Release processor from sleep
 *
 * @details
 * 	 This function call will release the processor from going into sleep mode while the peripheral is active.
 *
 * @note
 *   This function does not return any values.
 *
 * @param[in] EM
 *   Energy Mode to decrement
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]--;
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
	CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
 * @brief
 *	Enter sleep mode
 *
 * @details
 *	This function call will make the energy mode functionality atomic to protect lowest_energy_mode.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void enter_sleep(void) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	if (lowest_energy_mode[EM0] > 0) {
//		nothing
	}
	else if (lowest_energy_mode[EM1] > 0) {
//		nothing
	}
	else if (lowest_energy_mode[EM2] > 0) {
		EMU_EnterEM1();
	}
	else if (lowest_energy_mode[EM3] > 0) {
		EMU_EnterEM2(true);
	}
	else {
		EMU_EnterEM3(true);
	}

	CORE_EXIT_CRITICAL();
	return;
}

/***************************************************************************//**
 * @brief
 *	Returns energy mode
 *
 * @details
 *	This function call will return which energy mode the current system cannot enter.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void) {
	for(int i = 0; i < MAX_ENERGY_MODES; i++) {
		if (lowest_energy_mode[i] != 0) {
			return i;
		}
	}
	return (MAX_ENERGY_MODES - 1);
}
