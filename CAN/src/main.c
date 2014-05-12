//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "main.h"

// ----------------------------------------------------------------------------
//
// STM32F4 empty sample (trace via ITM).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the ITM output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// Variables
__IO uint8_t togglecounter = 0;
__IO uint8_t UserButtonPressed = 0;
int calledBack = 0;

void callback(char* buffer) {
	UART_put(buffer);
}

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed. // Which is 168MHz

	/* Configure LED3, LED4, LED5 & LED6 */
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
//	/* Configure user button */
//	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	// SysTick Config to 1ms
	SystemCoreClock = HAL_RCC_GetHCLKFreq();
	SysTick_Config(SystemCoreClock / 1000);

	// All ok : green led on
	BSP_LED_On(LEDGREEN);

	// Init CAN
	CAN_init(0, 0);

	// Send test message
	CAN_sendTest();

	// Start receiving
	CAN_receiveStart(&CAN_empty);

	// Define a message
	CanTxMsgTypeDef CanTxMsg;
	CanTxMsg.ExtId = 0;
	CanTxMsg.DLC = 8;
	CanTxMsg.IDE = CAN_ID_EXT;
	CanTxMsg.RTR = CAN_RTR_DATA;
	CanTxMsg.Data[0] = 'S';
	CanTxMsg.Data[1] = 'u';
	CanTxMsg.Data[2] = 'c';
	CanTxMsg.Data[3] = 'c';
	CanTxMsg.Data[4] = 'e';
	CanTxMsg.Data[5] = 's';
	CanTxMsg.Data[6] = 's';
	CanTxMsg.Data[7] = '!';

	// Main Loop
	while (1) {
		CAN_transmit(&CanTxMsg, 2);
		HAL_Delay(1000);
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
