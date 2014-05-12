#ifndef ___CAN___
#define ___CAN___

#include "main.h"

/* Use the LEDS ?? */
#define CAN_USE_LEDS 0

/* Timeout for each transmit attempt */
#define CAN_TX_TIMEOUT 100

/* Definition for CANx clock resources */
	// If using CAN2, CAN1 clock must be enabled too
#define CANx                           CAN1
#define CANx_CLK_ENABLE()              __CAN1_CLK_ENABLE()
#define CANx_RX_GPIO_CLK_ENABLE()      __GPIOD_CLK_ENABLE()
#define CANx_TX_GPIO_CLK_ENABLE()      __GPIOD_CLK_ENABLE()

#define CANx_FORCE_RESET()             __CAN1_FORCE_RESET()
#define CANx_RELEASE_RESET()           __CAN1_RELEASE_RESET()

/* Definition for CANx Pins */
#define CANx_TX_PIN                    GPIO_PIN_1
#define CANx_TX_GPIO_PORT              GPIOD
#define CANx_TX_AF                     GPIO_AF9_CAN1
#define CANx_RX_PIN                    GPIO_PIN_0
#define CANx_RX_GPIO_PORT              GPIOD
#define CANx_RX_AF                     GPIO_AF9_CAN1

/* Definition for CANx's NVIC */
#define CANx_RX0_IRQn                      CAN1_RX0_IRQn
#define CANx_IRQHandler                CAN1_IRQHandler

/* Exported functions */
	// Callback type for receiving a packet
	typedef void (*CAN_Callback_t)(CanRxMsgTypeDef*);
	void CAN_empty(CanRxMsgTypeDef* dontcare);

	// Receiving status
	typedef enum
	{
	  CAN_RX_STATUS_STOPPED             = 0x00,  /*!< No callback set, not calling callback */
	  CAN_RX_STATUS_PAUSED              = 0x01,  /*!< Callback set, not calling callback */
	  CAN_RX_STATUS_STARTED             = 0x02,  /*!< Callback set, Calling callback */
	  CAN_RX_STATUS_ERROR             = 0x03,  /*!< Callback is called but not set (set to empty placeholder) */
	} CanRxStatusTypeDef;

	// Transmitting status
	typedef enum
	{
	  CAN_TX_STATUS_READY				= 0x00,  /*!< Ready to send message */
	  CAN_TX_STATUS_BUSY				= 0x01,  /*!< A message is already sending, all other messages wait */
	  CAN_TX_STATUS_BLOCKED				= 0x02,  /*!< A message is already sending, all other messages wait */
	} CanTxStatusTypeDef;
	typedef enum
	{
	  CAN_TX_RETURN_DONE				= 0x00,  /*!< Done sending the message */
	  CAN_TX_RETURN_ERROR				= 0x01,  /*!< Error occured while sending */
	  CAN_TX_RETURN_BLOCKED				= 0x02,  /*!< Fatal error, all new transmition requests are refused */
	} CanTxReturnTypeDef;

// Initialization
void CAN_init(int8_t id, int8_t mask);
void CAN_setFilter(uint8_t num, uint8_t id, uint8_t mask, uint8_t active);
void HAL_CAN_MspInit(CAN_HandleTypeDef* handle);

// Transmission
void CAN_sendTest(void);
CanTxReturnTypeDef CAN_transmit(CanTxMsgTypeDef *message, uint8_t retry);
CanTxStatusTypeDef CAN_transmitStatus(void);

// Reception
void CAN_receiveStart(function_t callback);
void Can_receiveStop(void);
void Can_receivePause(void);
void Can_receiveUnPause(void);
CanRxStatusTypeDef CAN_receiveGetStatus(void);

// Internal
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan);
void HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan);

#endif
