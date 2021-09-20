/*
 * scheduler.c
 *
 *  Created on: Feb 11, 2021
 *      Author: adamp
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "scheduler.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Opens scheduler
 *
 * @details
 *	This function call will open the scheduler functionality by resetting the static variable event_scheduled to 0.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void scheduler_open(void) {
	event_scheduled = 0;
}


/***************************************************************************//**
 * @brief
 *   Adds an existing event
 *
 * @details
 * 	 This function call will OR a new event, the input argument, into the existing state of the static variable.
 *
 * @note
 *   This function does not return any values.
 *
 * @param[in] event
 *   New event.
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event) {
	event_scheduled |= event;
}


/***************************************************************************//**
 * @brief
 *   Removes an existing event from scheduler
 *
 * @details
 * 	 This function call will remove the event from the existing state of the static variable.
 *
 * @note
 *   This function does not return any values.
 *
 * @param[in] event
 *   Event to be removed.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event) {
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	event_scheduled &= ~event;

	CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
 * @brief
 *   Returns current state
 *
 * @details
 * 	 This function call will return the current state of the static variable event_scheduled.
 *
 * @note
 *   This function returns the current schduler state.
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void) {
	return event_scheduled;
}
