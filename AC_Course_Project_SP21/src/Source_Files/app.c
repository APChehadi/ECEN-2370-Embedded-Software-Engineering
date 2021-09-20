/**
 * @file
 * 	app.c
 * @author
 * 	Adam Chehadi
 * @date
 * 	02/07/2021
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include <stdio.h>
#include <stdlib.h>

//***********************************************************************************
// defined files
//***********************************************************************************
//#define BLE_TEST_ENABLED
#define TDD_TEST_ENABLED

//***********************************************************************************
// Static / Private Variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************
static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Set up structures
 *
 * @details
 *	These functions will be used to set up the structures that will be used by the peripheral's open driver functions.
 *
 * @note
 *	This function does not have any inputs or outputs.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	scheduler_open();
	sleep_open();
	sleep_block_mode(SYSTEM_BLOCK_EM);
	ble_open(BLE_TX_DONE_CB, BLE_RX_DONE_CB);
	add_scheduled_event(BOOT_UP_CB);
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	si7021_i2c_open();
	veml_i2c_open();
	veml_write();
}

/***************************************************************************//**
 * @brief
 *	Calls the letimer_pwm_open function
 *
 * @details
 *	This function call should set the structure defined in the letimer.h file with the desired
 *  values to enable proper PWM operation.
 *
 * @note
 *	This function does not return any values.
 *
 * @param[in]
 *	pwm period, pwm active period, address for LED0, address for LED1
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef app_letimer_pwm_struct;

	app_letimer_pwm_struct.debugRun = false;
	app_letimer_pwm_struct.enable = false;
	app_letimer_pwm_struct.out_pin_route0 = out0_route;
	app_letimer_pwm_struct.out_pin_route1 = out1_route;

	app_letimer_pwm_struct.out_pin_0_en = false;
	app_letimer_pwm_struct.out_pin_1_en = false;

	app_letimer_pwm_struct.period = period;
	app_letimer_pwm_struct.active_period = act_period;
	app_letimer_pwm_struct.uf_irq_enable = true;
	app_letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;
	app_letimer_pwm_struct.comp0_irq_enable = false;
	app_letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
	app_letimer_pwm_struct.comp1_irq_enable = false;
	app_letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;

	letimer_pwm_open(LETIMER0, &app_letimer_pwm_struct);
}


/***************************************************************************//**
 * @brief
 *	comp0 callback function
 *
 * @details
 *	This function call removes the comp0 event from the event scheduler.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void) {
	remove_scheduled_event(LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}


/***************************************************************************//**
 * @brief
 *	comp1 callback function
 *
 * @details
 *	This function call removes the comp1 event from the event scheduler.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void) {
	remove_scheduled_event(LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}


/***************************************************************************//**
 * @brief
 *	uf callback function
 *
 * @details
 *	This function call removes the uf event from the event scheduler.
 *
 * @note
 *	This function does not return any values.
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void) {
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_CB);
	remove_scheduled_event(LETIMER0_UF_CB);

	si7021_read(SI7021_READ_CB);
	veml_read(VEML_CB);
}


/***************************************************************************//**
 * @brief
 *	SI7021 humidity peripheral callback
 *
 * @details
 *	This SI7021 humidity done callback signals the completion of a humidity read from the peripheral.
 *	This callback has been added in the scheduler of the main.c's while(1) loop.
 *
 * @note
 *	This function does not have any input or return values.
 *
 ******************************************************************************/
void scheduled_si7021_humidity_cb(void) {
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_CB);
	remove_scheduled_event(SI7021_READ_CB);

	float returned_humidity = si7021_humidity_conversion();

	if (returned_humidity >= 30.0) {
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	} else {
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}

	char humidity_str[80];
	sprintf(humidity_str, "humidity = %.1f%%\n", returned_humidity);
	ble_write(humidity_str);

	si7021_temp_read(SI7021_READ_CB);
}


/***************************************************************************//**
 * @brief
 *	SI7021 humidity peripheral temperature data callback
 *
 * @details
 *	This SI7021 humidity done callback signals the completion of a humidity read from the peripheral.
 *	The temperature_calculation function is utilized to write the temperature.
 *	This callback has been added in the scheduler of the main.c's while(1) loop.
 *
 * @note
 *	This function does not have any input or return values.
 *
 ******************************************************************************/
void scheduled_si7021_temp_cb(void) {
	EFM_ASSERT(get_scheduled_events() & SI7021_TEMP_READ_CB);
	remove_scheduled_event(SI7021_TEMP_READ_CB);

	float returned_temperature = temperature_calculation();

	char temperature_str[80];
	sprintf(temperature_str, "temperature = %.1f F\n", returned_temperature);
	ble_write(temperature_str);
}


/***************************************************************************//**
 * @brief
 *	VEML read data callback
 *
 * @details
 *	This veml read callback signals the completion of a light sensor read from the peripheral.
 *	The compute_lux function is utilized to write the light value.
 *	This callback has been added in the scheduler of the main.c's while(1) loop.
 *
 * @note
 *	This function does not have any input or return values.
 *
 ******************************************************************************/
void scheduled_veml_read_cb(void) {
	EFM_ASSERT(get_scheduled_events() & VEML_CB);
	remove_scheduled_event(VEML_CB);

	float returned_lux = compute_lux();
	unsigned int unsigned_returned_lux = (unsigned int) returned_lux;
	char lux_str[80];
	sprintf(lux_str, "light = %i lux \n\n", unsigned_returned_lux);
	ble_write(lux_str);
}


/***************************************************************************//**
 * @brief
 *   callback function when system is booted up
 *
 * @details
 *   Function calls ble_test() to verify correct setup of LEUART (if BLE_TEST_ENABLED
 *   is defined from app.h), then calls ble_write()
 *   with "Hello World" to verify proper connection/printing ability to phone. Then calls
 *   letimer_start().
 *
 * @note
 *     no inputs, no outputs
 ******************************************************************************/
void scheduled_boot_up_cb(void) {
	EFM_ASSERT(get_scheduled_events() & BOOT_UP_CB);
	remove_scheduled_event(BOOT_UP_CB);

	#ifdef BLE_TEST_ENABLED
		bool ble_test_result = ble_test("Humidity");
		EFM_ASSERT(ble_test_result);

		timer_delay(DELAY);
	#endif

	#ifdef TDD_TEST_ENABLED
		bool tdd_test_result = i2c_test(SI7021_READ_CB);
		EFM_ASSERT(tdd_test_result);
		timer_delay(DELAY);
	#endif

	ble_write("\nHello World\n");
	letimer_start(LETIMER0, true);
}


/***************************************************************************//**
 * @brief
 *   callback function called after transmitting is done
 *
 * @details
 *   ASSERTS that this function was correctly called, then simply removes event
 *
 * @note
 *     no inputs, no outputs  (void func(void) {})
 ******************************************************************************/
void scheduled_ble_tx_done_cb(void) {
	EFM_ASSERT(get_scheduled_events() & BLE_TX_DONE_CB);
	remove_scheduled_event(BLE_TX_DONE_CB);
}
