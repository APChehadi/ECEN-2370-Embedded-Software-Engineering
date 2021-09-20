/**
 * @file
 * 	app.c
 * @author
 * 	Adam Chehadi
 * @date
 * 	02/25/2021
 * @brief
 *	Contains i2c_start, I2C0_IRQ_Handler, I2C1_IRQHandler, i2c_open, i2c_bus_reset, i2c_ack, i2c_nack, i2c_rxdatav, i2c_mstop
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
typedef enum {
	Start_Command,
	Read_Command,
	Write_Command,
	Wait_Read,
	End_Sensing,
	Stop
} DEFINED_STATES;

static I2C_STATE_MACHINE	i2c_state_machine_struct;
static I2C_STATE_MACHINE	veml_i2c_state_machine_struct;


//***********************************************************************************
// Private function prototypes
//***********************************************************************************
static void i2c_bus_reset(I2C_TypeDef * i2c);

static void i2c_ack(I2C_STATE_MACHINE *i2c_sm);
static void i2c_nack(I2C_STATE_MACHINE *i2c_sm);
static void i2c_rxdatav(I2C_STATE_MACHINE *i2c_sm);
static void i2c_mstop(I2C_STATE_MACHINE *i2c_sm);


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Start the I2C peripheral
 *
 * @details
 * 	 Initializes a private struct in i2c that will keep state of the progress of the I2C operation. This state information
 *   will be stored in a static struct in i2c.c of type I2C_STATE_MACHINE.
 *
 * @note
 *   This function does not have any return values.
 *
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2cx, uint32_t slave_address, uint32_t slave_register, bool read_write, uint32_t *data, uint32_t si_read_cb, uint32_t num_bytes) {
	EFM_ASSERT((I2C0->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);
	sleep_block_mode(I2C_EM_BLOCK);

	if(i2cx == I2C0) {
		veml_i2c_state_machine_struct.I2Cx = i2cx;
		veml_i2c_state_machine_struct.slave_address = slave_address;
		veml_i2c_state_machine_struct.slave_register = slave_register;
		veml_i2c_state_machine_struct.read_write = read_write;
		veml_i2c_state_machine_struct.num_transfer_bytes = num_bytes;
		veml_i2c_state_machine_struct.bytes_transfered = 0;
		veml_i2c_state_machine_struct.data = data;
		veml_i2c_state_machine_struct.si_cb = si_read_cb;
		veml_i2c_state_machine_struct.i2c_busy = true;
		veml_i2c_state_machine_struct.current_state = Start_Command;
		veml_i2c_state_machine_struct.I2Cx->CMD = I2C_CMD_START;
		veml_i2c_state_machine_struct.I2Cx->TXDATA = (veml_i2c_state_machine_struct.slave_address << 1) | I2C_WRITE;
	} else {
		i2c_state_machine_struct.I2Cx = i2cx;
		i2c_state_machine_struct.slave_address = slave_address;
		i2c_state_machine_struct.slave_register = slave_register;
		i2c_state_machine_struct.read_write = read_write;
		i2c_state_machine_struct.num_transfer_bytes = num_bytes;
		i2c_state_machine_struct.bytes_transfered = 0;
		i2c_state_machine_struct.data = data;
		i2c_state_machine_struct.si_cb = si_read_cb;
		i2c_state_machine_struct.i2c_busy = true;
		i2c_state_machine_struct.current_state = Start_Command;
		i2c_state_machine_struct.I2Cx->CMD = I2C_CMD_START;
		i2c_state_machine_struct.I2Cx->TXDATA = (i2c_state_machine_struct.slave_address << 1) | I2C_WRITE;
	}
}



/***************************************************************************//**
 * @brief
 *   checks whether the I2C state machine is currently running a communication protocol
 *
 * @details
 * 	 checks that the I2C typedef passed in matches the one currently being used by the I2C
 * 	 protocol, and if so returns the state machine "busy" element (will be either true = is busy,
 * 	 or false = is NOT busy), otherwise returns true so as to catch errors in code faster
 *
 * @param[in] i2c
 *   either I2C0 or I2C1 based upon which process calls the function
 ******************************************************************************/
bool check_busy(I2C_TypeDef * i2c) {
	if(i2c == i2c_state_machine_struct.I2Cx) {
		return i2c_state_machine_struct.i2c_busy;
	}
	else if(i2c == veml_i2c_state_machine_struct.I2Cx) {
		return veml_i2c_state_machine_struct.i2c_busy;
	} else {
		return true;
	}
}


/***************************************************************************//**
 * @brief
 *   I2C0 Interrupt Service Routine
 *
 * @details
 * 	 After each interrupt is received, the Interrupt Serviced Routine I2C0_IRQHandler will call the appropriate
 * 	 I2C static function to progress the state machine directly.
 *
 * @note
 *   This function does not have any input or return values.
 *
 ******************************************************************************/
void I2C0_IRQHandler(void) {
	uint32_t int_flag;
	int_flag = I2C0->IF & I2C0->IEN;
	I2C0->IFC = int_flag;

	if (int_flag & I2C_IF_ACK) {
		EFM_ASSERT(!(I2C0->IF & I2C_IF_ACK));
		i2c_ack(&veml_i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_NACK) {
		EFM_ASSERT(!(I2C0->IF & I2C_IF_NACK));
		i2c_nack(&veml_i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_RXDATAV) {
		i2c_rxdatav(&veml_i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_MSTOP) {
		EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));
		i2c_mstop(&veml_i2c_state_machine_struct);
	}
}


/***************************************************************************//**
 * @brief
 *   I2C1 Interrupt Service Routine
 *
 * @details
 * 	 After each interrupt is received, the Interrupt Serviced Routine I2C1_IRQHandler will call the appropriate
 * 	 I2C static function to progress the state machine directly.
 *
 * @note
 *   This function does not have any input or return values.
 *
 ******************************************************************************/
void I2C1_IRQHandler(void) {
	uint32_t int_flag;
	int_flag = I2C1->IF & I2C1->IEN;
	I2C1->IFC = int_flag;

	if (int_flag & I2C_IF_ACK) {
		EFM_ASSERT(!(I2C1->IF & I2C_IF_ACK));
		i2c_ack(&i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_NACK) {
		EFM_ASSERT(!(I2C1->IF & I2C_IF_NACK));
		i2c_nack(&i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_RXDATAV) {
		i2c_rxdatav(&i2c_state_machine_struct);
	}
	if (int_flag & I2C_IF_MSTOP) {
		EFM_ASSERT(!(I2C1->IF & I2C_IF_MSTOP));
		i2c_mstop(&i2c_state_machine_struct);
	}
}


/***************************************************************************//**
 * @brief
 *   I2C configure function
 *
 * @details
 * 	 The I2C driver must be configured to be modular and encapsulated meaning that it is completely contained and transportable.
 * 	 To support all the different possible i2c peripherals of the Pearl Gecko, there will be two i2c_open() function input arguments
 *
 * @note
 *   This function does not have any return values. The two input values provide the base address of which the I2C peripheral to be configured,
 *   and the struct that the device will use to define the Pearl Gecko I2C peripheral per the device requirements.
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef * i2c, I2C_OPEN_STRUCT * i2c_setup) {
	I2C_Init_TypeDef i2c_init_values;

	if(i2c == I2C0) {
		CMU_ClockEnable(cmuClock_I2C0, true);
	} else if (i2c == I2C1) {
		CMU_ClockEnable(cmuClock_I2C1, true);
	} else {
		EFM_ASSERT(false);
	}

	if ((i2c->IF & 0x01) == 0) {
		i2c ->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	} else {
		i2c->IFC = 0x01;
		EFM_ASSERT(!(i2c->IF & 0x01));
	}

	i2c_init_values.clhr = i2c_setup->clhr;
	i2c_init_values.enable = i2c_setup->enable;
	i2c_init_values.freq = i2c_setup->freq;
	i2c_init_values.master = i2c_setup->master;
	i2c_init_values.refFreq = i2c_setup->refFreq;

	I2C_Init(i2c, &i2c_init_values);

	i2c->ROUTELOC0 = i2c_setup->out_pin_scl_route | i2c_setup->out_pin_sda_route;
	i2c->ROUTEPEN = (I2C_ROUTEPEN_SCLPEN*i2c_setup->out_pin_scl_en) | (I2C_ROUTEPEN_SDAPEN*i2c_setup->out_pin_sda_en);

	i2c_bus_reset(i2c);

	if (i2c == I2C0) {
		I2C0->IFC = I2C_IF_ACK;
		I2C0->IEN |= I2C_IF_ACK;

		I2C0->IFC = I2C_IF_NACK;
		I2C0->IEN |= I2C_IF_NACK;

		I2C0->IFC = I2C_IF_MSTOP;
		I2C0->IEN |= I2C_IF_MSTOP;

//		I2C0->IFC = I2C_IF_RXDATAV;
		I2C0->IEN |= I2C_IF_RXDATAV;
	}

	if (i2c == I2C1) {
		I2C1->IFC = I2C_IF_ACK;
		I2C1->IEN |= I2C_IF_ACK;

		I2C1->IFC = I2C_IF_NACK;
		I2C1->IEN |= I2C_IF_NACK;

		I2C1->IFC = I2C_IF_MSTOP;
		I2C1->IEN |= I2C_IF_MSTOP;

//		I2C1->IFC = I2C_IF_RXDATAV;
		I2C1->IEN |= I2C_IF_RXDATAV;
	}

	if (i2c == I2C0) {
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	if (i2c == I2C1) {
		NVIC_EnableIRQ(I2C1_IRQn);
	}
}


//***********************************************************************************
// Private functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   I2C bus reset function
 *
 * @details
 * 	 A routine to reset the I2C state machines of the Pearl Gecko I2C peripheral as well as reset the I2C state machines of the external I2C
 * 	 devices such as the SI7021. This function will have one input arguments.
 *
 * @note
 *   This function does not have any return values. The one input value is I2C_TypeDef specifying which I2C internal peripheral.
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef * i2c) {
//	i2c->CMD = I2C_CMD_ABORT;

	uint32_t ien = i2c->IEN;
	i2c->IEN = 0;
	i2c->IFC = i2c->IF;
	i2c->CMD = I2C_CMD_CLEARTX;
	i2c->CMD = I2C_CMD_START | I2C_CMD_STOP;

	while(!(i2c->IF & I2C_IF_MSTOP));

	i2c->IFC = i2c->IF;
	i2c->IEN = ien;
	i2c->CMD = I2C_CMD_ABORT;
}


/***************************************************************************//**
 * @brief
 *   ack interrupt function
 *
 * @details
 * 	 This function is called when the system receives an ack interrupt.
 *
 * @note
 *   This function does not have any return values. The one input value is the state machine struct
 *
 ******************************************************************************/
void i2c_ack(I2C_STATE_MACHINE * i2c_sm) {
	switch(i2c_sm->current_state) {
		case Start_Command:
			if(i2c_sm->read_write == I2C_READ) {
				i2c_sm->current_state = Read_Command;
				i2c_sm->I2Cx->TXDATA = i2c_sm->slave_register; //from J
			}
			else {
				i2c_sm->current_state = Write_Command;
				i2c_sm->I2Cx->TXDATA = i2c_sm->slave_register;
			}
			break;
		case Read_Command:
			i2c_sm->current_state = Wait_Read;
			i2c_sm->I2Cx->CMD = I2C_CMD_START;
			i2c_sm->I2Cx->TXDATA = ((i2c_sm->slave_address << 1) | I2C_READ); //from J
			break;
		case Write_Command:
			if(i2c_sm->num_transfer_bytes == 1) {
				i2c_sm->current_state = End_Sensing;
				i2c_sm->I2Cx->TXDATA = *(i2c_sm->data);
			}
			else if (i2c_sm->num_transfer_bytes == 2) {
				if(i2c_sm->bytes_transfered == 0) {
					i2c_sm->bytes_transfered = 1;
					i2c_sm->I2Cx->TXDATA = *(i2c_sm->data);
				}
				else if(i2c_sm->bytes_transfered == 1) {
					i2c_sm->current_state = End_Sensing;
					i2c_sm->I2Cx->TXDATA |= *(i2c_sm->data) << (8 * i2c_sm->num_transfer_bytes-1);
				}
			}
			break;
		case Wait_Read:
			i2c_sm->current_state = End_Sensing;
			break;
		case End_Sensing:
			if(i2c_sm->read_write == I2C_WRITE) {
				i2c_sm->current_state = Stop;
				i2c_sm->I2Cx->CMD = I2C_CMD_STOP;
			}
			else {
				EFM_ASSERT(false);
			}
			break;
		case Stop:
		default:
			EFM_ASSERT(false);
	}
}


/***************************************************************************//**
 * @brief
 *   nack interrupt function
 *
 * @details
 * 	 This function is called when the system receives an nack interrupt.
 *
 * @note
 *   This function does not have any return values. The one input value is the state machine struct
 *
 ******************************************************************************/
void i2c_nack(I2C_STATE_MACHINE * i2c_sm) {
	switch(i2c_sm->current_state) {
		case Start_Command:
			EFM_ASSERT(false);
			break;
		case Read_Command:
			EFM_ASSERT(false);
			break;
		case Write_Command:
			EFM_ASSERT(false);
			break;
		case Wait_Read:
			i2c_sm->current_state = Wait_Read;
			i2c_sm->I2Cx->CMD = I2C_CMD_START;
			i2c_sm->I2Cx->TXDATA = ((i2c_sm->slave_address << 1) | I2C_READ);
			break;
		case End_Sensing:
		case Stop:
		default:
			EFM_ASSERT(false);
	}
}


/***************************************************************************//**
 * @brief
 *   rxdatav interrupt function
 *
 * @details
 * 	 This function is called when the system receives an rxdatav interrupt.
 *
 * @note
 *   This function does not have any return values. The one input value is the state machine struct
 *
 ******************************************************************************/
void i2c_rxdatav(I2C_STATE_MACHINE * i2c_sm) {
	switch(i2c_sm->current_state) {
		case Start_Command:
			EFM_ASSERT(false);
			break;
		case Read_Command:
			EFM_ASSERT(false);
			break;
		case Wait_Read:
			EFM_ASSERT(false);
			break;
		case Write_Command:
			EFM_ASSERT(false);
			break;
		case End_Sensing:
			i2c_sm->num_transfer_bytes -= 1;
			if(i2c_sm->I2Cx == i2c_state_machine_struct.I2Cx)
				if(i2c_sm->num_transfer_bytes > 0) {
					*i2c_sm->data = (i2c_sm->I2Cx->RXDATA) << (8 * i2c_sm->num_transfer_bytes);
					i2c_sm->I2Cx->CMD = I2C_CMD_ACK;
				}
				else {
					*i2c_sm->data |= i2c_sm->I2Cx->RXDATA;
					i2c_sm->current_state = Stop;
					i2c_sm->I2Cx->CMD = I2C_CMD_NACK;
					i2c_sm->I2Cx->CMD = I2C_CMD_STOP;
				}
			else {
				if(i2c_sm->num_transfer_bytes > 0) {
					*i2c_sm->data = i2c_sm->I2Cx->RXDATA;
					i2c_sm->I2Cx->CMD = I2C_CMD_ACK;
				}
				else {
					*i2c_sm->data |= (i2c_sm->I2Cx->RXDATA) << 8;
					i2c_sm->current_state = Stop;
					i2c_sm->I2Cx->CMD = I2C_CMD_NACK;
					i2c_sm->I2Cx->CMD = I2C_CMD_STOP;
				}
			}
			break;
		case Stop:
		default:
			EFM_ASSERT(false);
	}
}

/***************************************************************************//**
 * @brief
 *   mstop interrupt function
 *
 * @details
 * 	 This function is called when the system receives an mstop interrupt.
 *
 * @note
 *   This function does not have any return values. The one input value is the state machine struct
 *
 ******************************************************************************/
void i2c_mstop(I2C_STATE_MACHINE * i2c_sm) {
	switch(i2c_sm->current_state) {
		case Start_Command:
			EFM_ASSERT(false);
			break;
		case Read_Command:
			EFM_ASSERT(false);
			break;
		case Wait_Read:
			EFM_ASSERT(false);
			break;
		case Write_Command:
			EFM_ASSERT(false);
			break;
		case End_Sensing:
			EFM_ASSERT(false);
			break;
		case Stop:
			sleep_unblock_mode(I2C_EM_BLOCK);
			add_scheduled_event(i2c_sm->si_cb);
			i2c_sm->current_state = Start_Command;
			i2c_sm->i2c_busy = false;
			break;
		default:
			EFM_ASSERT(false);
	}
}
