/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef TXHeader;
CAN_RxHeaderTypeDef RXHeader;

uint32_t pTxMailbox = 0;
uint32_t CAN_ID = 0x01;
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 12;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_Config(void)
{
	CAN_FilterTypeDef  sFilterConfig;
 
  sFilterConfig.FilterBank = 0;                     
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;           
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;    
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;        
  sFilterConfig.SlaveStartFilterBank = 14;
	
	if(HAL_CAN_ConfigFilter(&hcan,&sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	if(HAL_CAN_Start(&hcan) != HAL_OK)//打开can
	{
		Error_Handler();
	}
	if(HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		Error_Handler();
	}
	
  TXHeader.StdId=CAN_ID;
  TXHeader.ExtId = 0x12345000;
  TXHeader.RTR = CAN_RTR_DATA;
  TXHeader.DLC = 8;
  TXHeader.TransmitGlobalTime = DISABLE;
}

void CAN_senddata(uint8_t* msg,uint8_t len)
{
	uint8_t i=0;
	uint8_t message[8];
 
	TXHeader.ExtId=0xAAAAA000;       
	TXHeader.IDE=CAN_ID_EXT;    
	TXHeader.RTR=CAN_RTR_DATA;  
	TXHeader.DLC=len;    
	
  for(i=0;i<len;i++)
  {
		message[i]=msg[i];
	}
	
  if(HAL_CAN_AddTxMessage(&hcan, &TXHeader, message, &pTxMailbox) != HAL_OK)
	{
		UART1_printf("CAN sending failed.\n\n");
		return;
	}
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3) {}
  return;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanNum)
{
	uint32_t i;
	uint8_t	RxData[8];
	
	//Rx_Flag =1;
	HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RXHeader, RxData);
	UART1_printf("\nFrom CAN in loopback mode.\n");
	for(i=0;i<RXHeader.DLC;i++)
	{
		UART1_printf("%c", RxData[i]);
	}
	UART1_printf("\n\n");
		//RxBuf[i]=RxData[i];
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
