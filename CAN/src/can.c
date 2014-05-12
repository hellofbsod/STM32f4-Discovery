/*
 * High-level library to send and receive messages through a CAN link
 *
 * ### Quick start
 *  // Init CAN
 *	CAN_init(0, 0);
 *
 *	// Send test message
 *	CAN_sendTest();
 *
 *	// Start receiving
 *	CAN_receiveStart(&CAN_empty);
 *
 *	// Define a message
 *	CanTxMsgTypeDef CanTxMsg;
 *	CanTxMsg.ExtId = 0;
 *	CanTxMsg.DLC = 8;
 *	CanTxMsg.IDE = CAN_ID_EXT;
 *	CanTxMsg.RTR = CAN_RTR_DATA;
 *	CanTxMsg.Data[0] = 'S';
 *	CanTxMsg.Data[1] = 'u';
 *	CanTxMsg.Data[2] = 'c';
 *	CanTxMsg.Data[3] = 'c';
 *	CanTxMsg.Data[4] = 'e';
 *	CanTxMsg.Data[5] = 's';
 *	CanTxMsg.Data[6] = 's';
 *	CanTxMsg.Data[7] = '!';
 *
 *	// Main Loop
 *	while (1) {
 *		CAN_transmit(&CanTxMsg, 2);
 *		HAL_Delay(1000);
 *	}
 *
 *
 *
 * ### PARAMS IN .h
 * 		- CAN_USE_LEDS defines wether or not the library uses the leds to mark its state
 * 			LED Code
 * 				Slow Blink :
 *					- RED + ALL REST ON : Fatal error, execution stopped
 *
 *				Fast Blink :
 *					- Blue : CAN Tx activity (1 message = 1 toggle)
 *					- Green : CAN Rx activity
 *					- Orange : CAN Link Errors (1 error = 1 toggle)
 *					- Red : CAN Busy-Waiting for link to be ready, Rx or Tx, (10ms waiting = 1 toggle),
 *							or Transmission retrying (anyway it means delay)
 *
 *		- Clock, Pin, AF, GPIO params to select the pins and the CAN instance. CAN1_CLK must always be enabled even if only using CAN2
 *
 *	### Initialization
 *		- CAN_init()
 *			Init the CAN instance and set the GPIO pins. User can provide an initial filter, or (-1, -1) to not set any filter
 *		- CAN_setFilter()
 *			Set a filter, with its @num (0..27) , @id (extended 0x0000..0xFFFF), @mask, @active (0, 1)
 *
 *		Warning: If no filters are set, no message will be received !
 *
 *	### Transmitting
 *		- CAN_sendTest()
 *			Sends a test message, id:0 (ext), message: "TesTTesT"
 *		- CAN_transmit()
 *			Sends a message provided using HAL structures, and a max numer of retries before giving up sending the packet
 *			Returns a value in the typedef enum, being OK ERROR or BLOCKED (if a fatal eror caused the library to lock any transmission)
 *		- CAN_transmitStatus()
 *			Returns the current Tx status (library internal)
 *
 *	### Receiving
 *		Receiving is acheived through IRQ interrupts and callbacks, user must provide a callback called upon reception of a new message that passes the HardFilters
 *
 *		- CAN_receiveStart()
 *			Starts to listen for new messages, user provides a callback taking as parameter a CanRxMsgTypeDef*
 *		- CAN_receivePause()
 *			Stops listening but keeps in memory the callback to fast-restart
 *		- CAN_receiveUnPause()
 *			Fast-restart, can only be called after a pause
 *		- CAN_receiveStop()
 *			Stops listening and resets the callback to its default placeholder
 *		- CAN_receiveStatus()
 *			Return the current receiving status
 *			STARTED, STOPPED, PAUSED, ERROR (if locked by the library upon fatal error)
 *
 */
#include "main.h"

// Global Vars
CanRxStatusTypeDef CanRxStatus = CAN_RX_STATUS_STOPPED;
CanTxStatusTypeDef CanTxStatus = CAN_TX_STATUS_READY;
CAN_HandleTypeDef CanHandle;

// Callback for receiving
void CAN_empty(CanRxMsgTypeDef* dontcare) {}
CAN_Callback_t CanRxCallback = &CAN_empty;

// Static function for fatal error
static void Error_Handler(void);


/*
 * Init the CAN link with given filter as filter 0,
 * initialise with -1,-1 if you don't want to set any filter now
 */
void CAN_init(int8_t id, int8_t mask) {

	/* CAN cell init */
	CanHandle.Instance = CANx;
	CanHandle.Init.TTCM = DISABLE; // time-triggered communication mode = DISABLED
	CanHandle.Init.ABOM = DISABLE; // automatic bus-off management mode = DISABLED
	CanHandle.Init.AWUM = DISABLE; // automatic wake-up mode = DISABLED
	CanHandle.Init.NART = DISABLE; // non-automatic retransmission mode = DISABLED
	CanHandle.Init.RFLM = DISABLE; // receive FIFO locked mode = DISABLED
	CanHandle.Init.TXFP = DISABLE; // transmit FIFO priority = DISABLED
	CanHandle.Init.Mode = CAN_MODE_NORMAL;

	/* CAN Baudrate = 1 MBps (CAN clocked at 42 MHz, not at 30 like on the other examples !!!!) */
	// CAN Bitrate
	CanHandle.Init.SJW = CAN_SJW_1TQ;    // SJW (1 bis 4 möglich)
	CanHandle.Init.BS1 = CAN_BS1_14TQ;   // Samplepoint 72%
	CanHandle.Init.BS2 = CAN_BS2_6TQ;    // Samplepoint 72%

//	CanHandle.Init.Prescaler = 1;        // 2000 kbit/s
	CanHandle.Init.Prescaler = 2;        // 1000 kbit/s
//	CanHandle.Init.Prescaler = 4;        //  500 kbit/s
//	CanHandle.Init.Prescaler = 5;        //  400 kbit/s
//	CanHandle.Init.Prescaler = 8;        //  250 kbit/s
//	CanHandle.Init.Prescaler = 10;       //  200 kbit/s
//	CanHandle.Init.Prescaler = 16;       //  125 kbit/s
//	CanHandle.Init.Prescaler = 20;       //  100 kbit/s
//	CanHandle.Init.Prescaler = 40;       //   50 kbit/s
//	CanHandle.Init.Prescaler = 80;       //   40 kbit/s
//	CanHandle.Init.Prescaler = 200;      //   10 kbit/s


	/* HAL init the CAN bus */
	if (HAL_CAN_Init(&CanHandle) != HAL_OK) {
		Error_Handler();
	}

	/* Set the CAN filter to given initial filter */
	if (id >= 0 && mask >= 0) CAN_setFilter(0, id, mask, 1);

}
void CAN_setFilter(uint8_t num, uint8_t id, uint8_t mask, uint8_t active) {
	CAN_FilterConfTypeDef FilterConf;

	FilterConf.FilterNumber = num;
	FilterConf.FilterMode = CAN_FILTERMODE_IDMASK;
	FilterConf.FilterScale = CAN_FILTERSCALE_32BIT;
	FilterConf.FilterIdHigh = id;
	FilterConf.FilterIdLow = id;
	FilterConf.FilterMaskIdHigh = mask;
	FilterConf.FilterMaskIdLow = mask;
	FilterConf.FilterFIFOAssignment = 0;
	FilterConf.FilterActivation = (active)?ENABLE:DISABLE;
	FilterConf.BankNumber = 14;

	if(HAL_CAN_ConfigFilter(&CanHandle, &FilterConf) != HAL_OK)
	{
		/* Filter configuration Error */
		Error_Handler();
	}
}

/*
 * Init the CAN GPIO
 */
void HAL_CAN_MspInit(CAN_HandleTypeDef* handle) {
	/* Init CAN clock(s) */
	CANx_CLK_ENABLE();
	CANx_RX_GPIO_CLK_ENABLE();
	CANx_TX_GPIO_CLK_ENABLE();

	/* GPIO Structure */
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
			// RX
	GPIO_InitStructure.Pin = CANx_RX_PIN;
	GPIO_InitStructure.Alternate = CANx_RX_AF;
	HAL_GPIO_Init(CANx_RX_GPIO_PORT, &GPIO_InitStructure);
			//TX
	GPIO_InitStructure.Pin = CANx_TX_PIN;
	GPIO_InitStructure.Alternate = CANx_TX_AF;
	HAL_GPIO_Init(CANx_TX_GPIO_PORT, &GPIO_InitStructure);

	/* NVIC Interrupt Config */
	HAL_NVIC_EnableIRQ(CANx_RX0_IRQn);
	HAL_NVIC_SetPriority(CANx_RX0_IRQn, 0x00, 0x00);

}

/*
 * Sends a test packet using CAN
 */
void CAN_sendTest(void) {
	CanTxMsgTypeDef txMessage;
	txMessage.StdId = 0x00;
	txMessage.ExtId = 0x00;
	txMessage.RTR = CAN_RTR_DATA;
	txMessage.IDE = CAN_ID_EXT;
	txMessage.DLC = 8;
	txMessage.Data[0] = 'T';
	txMessage.Data[1] = 'e';
	txMessage.Data[2] = 's';
	txMessage.Data[3] = 'T';
	txMessage.Data[4] = 'T';
	txMessage.Data[5] = 'e';
	txMessage.Data[6] = 's';
	txMessage.Data[7] = 'T';

	CAN_transmit(&txMessage, 2);
}

/*
 * Emits a message on CAN bus
 *
 * If timeout (100ms given) or link busy, retries up to retry times
 * If error, locks the link
 * If retry limit reached, returns error
 */
CanTxReturnTypeDef CAN_transmit(CanTxMsgTypeDef *message, uint8_t retry) {
	if (CanTxStatus == CAN_TX_STATUS_BLOCKED) return CAN_TX_RETURN_BLOCKED;
	else {
		// Wait for the link to free
		while (CanTxStatus != CAN_TX_STATUS_READY) {
			if (CAN_USE_LEDS) {
				BSP_LED_Toggle(LEDRED);
				HAL_Delay(10);
			}
		}

		// Take the lock
		CanTxStatus = CAN_TX_STATUS_BUSY;

		// Set the message into the handle
		CanHandle.pTxMsg = message;

		// Send the message as many times as requested
		uint8_t loop = 1;
		uint8_t count = 0;
		CanTxReturnTypeDef retval = CAN_TX_RETURN_ERROR;
		while (loop && count <= retry) {
			switch(HAL_CAN_Transmit(&CanHandle, CAN_TX_TIMEOUT)) {

				case (HAL_OK): // Message sent
					loop = 0;
					retval = CAN_TX_RETURN_DONE;

					// Toggle emitting LED
					if (CAN_USE_LEDS) BSP_LED_Toggle(LEDBLUE);

					break;

				case (HAL_BUSY): // Link busy (should never be returned, but added for the sake of standards
				case (HAL_TIMEOUT): // Message timed out
					BSP_LED_Toggle(LEDRED);
					count++;
					break;

				case (HAL_ERROR):
						/* The only possible problem is that no mailboxes are available,
						 * we could just retry but the HAL is coded so that
						 * everything related to link status is rewritten with error status
						 * therefore we should lock the link to avoid other errors
						 */
					loop = 0;
					retval = CAN_TX_RETURN_ERROR;
					CanTxStatus = CAN_TX_STATUS_BLOCKED;
					break;

				default: // Unhandle return value, should never happen
					loop = 0;
					retval = CAN_TX_RETURN_ERROR;

			}

		}
		// If not locked, free the lock
		if (CanTxStatus != CAN_TX_STATUS_BLOCKED) CanTxStatus = CAN_TX_STATUS_READY;

		// Return the result
		return retval;
	}
}
CanTxStatusTypeDef CAN_transmitStatus(void) {
	return CanTxStatus;
}

/*
 * Handling the reception of messages
 */
void _CAN_rxStart(void) {
	if (HAL_CAN_Receive_IT(&CanHandle, CAN_FIFO0) != HAL_OK) {
		Error_Handler();
	}
}
void Can_receiveStop(void) {
	// Remove the callback
	if (CanRxStatus != CAN_RX_STATUS_ERROR) {
		CanRxCallback = &CAN_empty;
		CanRxStatus = CAN_RX_STATUS_STOPPED;
	}
}
void CAN_receiveStart(function_t callback) {
	if (CanRxStatus != CAN_RX_STATUS_ERROR) {
		// Remove all callbacks
		Can_receiveStop();

		// Add the new callback
		CanRxCallback = callback;
		CanRxStatus = CAN_RX_STATUS_STARTED;

		_CAN_rxStart();
	}
}
void Can_receivePause(void) {
	// Suspend the loop
	if (CanRxStatus == CAN_RX_STATUS_STARTED) {
		CanRxStatus = CAN_RX_STATUS_PAUSED;
	}
}
void Can_receiveUnPause(void) {
	// Relaunch the loop
	if (CanRxStatus == CAN_RX_STATUS_PAUSED) {
		CanRxStatus = CAN_RX_STATUS_STARTED;
		_CAN_rxStart();
	}
}
CanRxStatusTypeDef CAN_receiveGetStatus(void) {
	return CanRxStatus;
}

/*
 * Handles the Reception Event
 */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	if (CanRxStatus == CAN_RX_STATUS_STARTED) {
		// Toggle receiving LED
		if (CAN_USE_LEDS) BSP_LED_Toggle(LEDGREEN);

		// Call the Callback
		CanRxCallback(hcan->pRxMsg);

		// Relaunch the loop
		_CAN_rxStart();
	}
}

/*
 * Handles the link error event
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan) {
	// Toggle error LED
	if (CAN_USE_LEDS) BSP_LED_Toggle(LEDORANGE);
}

/*
 * We don't use IT for emitting, the led toggle is therefore useless here and defined right in the transmission section
 */
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
	// Toggle emitting LED
	if (CAN_USE_LEDS) BSP_LED_Toggle(LEDBLUE);
}


/*
  *  Error Handler
  */
 static void Error_Handler(void)
 {
	 // Lock the links
	 CanRxStatus = CAN_RX_STATUS_ERROR;
	 CanTxStatus = CAN_TX_STATUS_BLOCKED;

 	/* Blink LED5 (RED), all LEDS on, on error */
	BSP_LED_On(LEDGREEN);
	BSP_LED_On(LEDBLUE);
	BSP_LED_On(LEDORANGE);
	while(1)
	{
		BSP_LED_Toggle(LEDRED);
		HAL_Delay(1000);
	}
 }
