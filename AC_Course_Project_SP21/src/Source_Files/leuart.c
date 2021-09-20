/**
 * @file leuart.c
 * @author
 * @date
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_cb;
uint32_t	tx_done_cb;
bool		leuart0_tx_busy;

static LEUART_STATE_MACHINE			leuart_state_struct;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void leuart_txbl(LEUART_STATE_MACHINE *leuart_state);
static void leuart_txc(LEUART_STATE_MACHINE *leuart_state);


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
* @brief
*   Driver to open the leuart protocol
*
* @details
*      The driver enables the correct clock (for LEUARTx, based on what is passed in),
*      and sets the values for the leuart to use (baudrate, etc), as well as initializes
*      the leuart, enables interrupts, and enables the leuart
*
* @note
*   Will enable leuart for LEUARTx based on what is passed in
*
* @param[in] leuart
*   LEUARTx based on what application called this driver
*
* @param[in] leuart_settings
*   LEUART_OPEN_STRUCT with values for specific application
*
******************************************************************************/
void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	LEUART_Init_TypeDef leuart_values;

	if(leuart == LEUART0) {
		CMU_ClockEnable(cmuClock_LEUART0, true);
		NVIC_EnableIRQ(LEUART0_IRQn);
	} else {
		EFM_ASSERT(false);
	}

	if(!(leuart->STARTFRAME & 0x01)) {
		leuart->STARTFRAME = 0x01;
		while(leuart->SYNCBUSY);
		EFM_ASSERT(leuart->STARTFRAME & 0x01);
		leuart->STARTFRAME = 0x0;
		while(leuart->SYNCBUSY);
	}

	leuart_values.refFreq = 0;
	leuart_values.baudrate = leuart_settings->baudrate;
	leuart_values.databits = leuart_settings->databits;
	leuart_values.parity = leuart_settings->parity;
	leuart_values.stopbits = leuart_settings->stopbits;
	leuart_values.enable = leuartDisable;

	// Init
	LEUART_Init(leuart, &leuart_values);
	while(leuart->SYNCBUSY);

	// Route
	leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc;
	leuart->ROUTEPEN = (leuart_settings->tx_pin_en * leuart_settings->tx_en) | (leuart_settings->rx_pin_en * leuart_settings->rx_en);

	// Clear RX/TX Buffers
	leuart->CMD = LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;
	while(leuart->SYNCBUSY);

	if(leuart_settings->rx_en) {
		leuart->CMD = LEUART_CMD_RXEN;
		while(!(leuart->STATUS & LEUART_STATUS_RXENS));
		EFM_ASSERT(leuart->STATUS & LEUART_STATUS_RXENS);
	}

	if(leuart_settings->tx_en) {
		leuart->CMD = LEUART_CMD_TXEN;
		while(!(leuart->STATUS & LEUART_STATUS_TXENS));
		EFM_ASSERT(leuart->STATUS & LEUART_STATUS_TXENS);
	}

	tx_done_cb = leuart_settings->tx_done_evt;
	rx_done_cb = leuart_settings->rx_done_evt;

	LEUART_Enable(leuart, leuart_settings->enable);

	// Clear all interrupts
	leuart->IFC = _LEUART_IFC_MASK;
}


/***************************************************************************//**
 * @brief
 *   handles interrupts of the LEUART0
 *
 * @details
 *      calls specific (private) ISR functions based on the interrupt
 *
 * @note
 *   EFM_ASSERT() is called to verify that events are not being added if their flags
 *   haven't been set
 *
 *   no inputs, no outputs  (void func(void) {})
 *
 ******************************************************************************/
void LEUART0_IRQHandler(void){
	uint32_t int_flag;

	int_flag = LEUART0->IF & LEUART0->IEN;

	LEUART0->IFC = int_flag;

	if(int_flag & LEUART_IF_TXBL) {
		leuart_txbl(&leuart_state_struct);
	}
	if(int_flag & LEUART_IF_TXC) {
		leuart_txc(&leuart_state_struct);
	}
}

/***************************************************************************//**
 * @brief
 *   initializes everything for the leuart protocol, then
 *
 * @details
 *     starts the leuart protocol by initializing the parameters of the leuart state machine,
 *     and then enabling the TXBL interrupt
 *
 * @note
 *      waits until leuart is not busy, then disables all interrupts so that the optimizer
 *      does not change the order and enable the TXBL interrupt before initializing the other
 *      settings
 *
 * @param[in] leuart
 *   LEUARTx based on specific application code that called this driver
 *
 * @param[in] *string
 *   pointer to string for data to transmit
 *
 * @param[in] string_len
 *   length of *string, used to know when full sequence has been transmitted
 *
 * @param[in] CB
 *   callback definition for when to use when transmitting is done

 ******************************************************************************/
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	while(leuart->SYNCBUSY);

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	sleep_block_mode(LEUART_TX_EM);

	leuart_state_struct.state = EnableTransfer;
	leuart_state_struct.leuart = leuart;
	leuart_state_struct.count = 0;
	leuart_state_struct.length = string_len;
	leuart_state_struct.callback = tx_done_cb;
	strcpy(leuart_state_struct.string, string);

	leuart_state_struct.busy = true;
	LEUART0->IEN |= LEUART_IEN_TXBL;

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   checks whether the LEUART state machine is currently running a communication protocol
 *
 * @details
 * 	 returns the state machine "busy" element (will be either true = is busy,
 * 	 or false = is NOT busy)
 ******************************************************************************/
bool leuart_tx_busy(LEUART_TypeDef *leuart){
//	return !(leuart->STATUS & LEUART_STATUS_TXIDLE);
	return leuart_state_struct.busy;
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){
	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}


/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/
void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}



//***********************************************************************************
// Private functions
//***********************************************************************************

/***************************************************************************//**
* @brief
*   handles the "txbl" interrupt for the various cases (different states)
*   during the leuart transfer process
*
* @details
*      handles the TXBL interrupt based on the state, and produces EFM_ASSERT(false) if
*      an TXBL occurred in a state where it should not have
*
* @note
*   state machine is passed in so can be used by any LEUARTx
*
* @param[in] leuart_stmc
*   LEUARTx (x based on what is calling this driver)
*
******************************************************************************/
void leuart_txbl(LEUART_STATE_MACHINE *leuart_state) {
	switch(leuart_state->state) {
		case EnableTransfer: {
			leuart_state->state = TransferCharacters;
			break;
		}
		case TransferCharacters: {
			if(leuart_state->count < leuart_state->length) {
				leuart_state->leuart->TXDATA = leuart_state->string[leuart_state->count];
				leuart_state->count = leuart_state->count + 1;
				leuart_state->state = TransferCharacters;
			}
			if(leuart_state->count == leuart_state->length) {
				LEUART_IntDisable(leuart_state->leuart, LEUART_IF_TXBL);
				LEUART_IntEnable(leuart_state->leuart, LEUART_IF_TXC);
				leuart_state->state = EndTransfer;
			}
			break;
		}
		case EndTransfer: {
			// impossible
			EFM_ASSERT(false);
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}


/***************************************************************************//**
* @brief
*   handles the "txc" interrupt for the various cases (different states)
*   during the leuart transfer process
*
* @details
*      handles the TXC interrupt based on the state, and produces EFM_ASSERT(false) if
*      an TXC occurred in a state where it should not have
*
* @note
*   state machine is passed in so can be used by any LEUARTx
*
* @param[in] leuart_stmc
*   LEUARTx (x based on what is calling this driver)
*
******************************************************************************/
void leuart_txc(LEUART_STATE_MACHINE *leuart_state) {
	switch(leuart_state->state) {
		case EnableTransfer: {
			// impossible
			EFM_ASSERT(false);
			break;
		}
		case TransferCharacters: {
			// impossible
			EFM_ASSERT(false);
			break;
		}
		case EndTransfer: {
			LEUART_IntDisable(leuart_state->leuart, LEUART_IF_TXC);
			sleep_unblock_mode(LEUART_TX_EM);
			add_scheduled_event(leuart_state->callback);
			leuart_state->state = EnableTransfer;
			leuart_state->busy = false;
			break;
		}
		default: {
			EFM_ASSERT(false);
			break;
		}
	}
}






