/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_client.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the ASU client initialization and generic queue management functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/03/24 Initial release
 *       ma   07/17/24 Update P0, P1 Queue addresses and set IsCmdPresent to TRUE before triggering
 *                     the IPI interrupt to ASU
 *       ss   08/13/24 Changed XAsu_ClientInit function prototype and Initialized mailbox in
 *                     XAsu_ClientInit() API
 *       ss   09/19/24 Added XAsu_CheckAsufwPrsntBit() API
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_status.h"

/************************************ Constant Definitions ***************************************/
/* TODO the shared memory shall be coming as part of design */
#define XASU_SHARED_MEMORY_P0_CH_QUEUE  0xEBE415B8U /** P0 queue shared memory */
#define XASU_SHARED_MEMORY_P1_CH_QUEUE  0xEBE41ADCU /**< P1 queue shared memory */

#define XASU_QUEUE_BUFFER_FULL          0xFFU       /**< To indicate queue full state */
#define XASU_CLIENT_READY               0xFFFFFFFFU /**< To indicate Client is ready */
#define XASU_TARGET_IPI_INT_MASK        1U          /**< ASU IPI interrupt mask */

#define ASU_GLOBAL_BASEADDR             (0xEBF80000U) /**< ASU GLOBAL register base address */
#define ASU_GLOBAL_GLOBAL_CNTRL         (ASU_GLOBAL_BASEADDR + 0x00000000U) /**< ASU GLOBAL CNTRL
                                                                             register address */

#define ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK       0x10U          /**< ASU FW Present mask
                                                                              value */
#define XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE	0xFFFFFU	/**< ASUFW check timoeout value */

/************************************** Type Definitions *****************************************/
/*
 * This typedef contains all the parameters required to manage the client library
 * Also it holds the shared memory queue index details
 */
typedef struct {
	XMailbox *MailboxPtr;
	XAsu_QueueInfo P0Queue;
	XAsu_QueueInfo P1Queue;
	u32 IsReady;
} XAsu_Client;
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XAsu_Client *XAsu_GetClientInstance(void);
static u32 XAsu_IsChannelQueueFull(XAsu_QueueInfo *QueueInfo);
static s32 XAsu_SendIpi(void);
static void XAsu_DoorBellToClient(void *CallBackRef);
static s32 XAsu_CheckAsufwPrsntBit(void);

/************************************ Variable Definitions ***************************************/
static XMailbox MailboxInstance;        /**< Variable to Mailbox instance */
static volatile u32 RecvDone = FALSE;	/**< Done flag */

/*************************************************************************************************/
/**
*
* @brief This function initializes the client instance.
*
* @param    DeviceId    The IPI Instance to be worked on
*
* @return
* 	        - XST_SUCCESS	On successful initialization
* 	        - XST_FAILURE	On failure
*
*************************************************************************************************/
s32 XAsu_ClientInit(u8 DeviceId)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();

	/* If already initialized returns success as no initialization is needed */
	if (ClientInstancePtr->IsReady == XASU_CLIENT_READY) {
		Status = XST_SUCCESS;
		goto END;
	}

	Status = XAsu_CheckAsufwPrsntBit();
	if (Status != XST_SUCCESS) {
		Status = XASU_ASUFW_NOT_PRESENT;
		goto END;
	}

	Status = (s32)XMailbox_Initialize(&MailboxInstance, DeviceId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ClientInstancePtr->MailboxPtr = &MailboxInstance;
	ClientInstancePtr->P0Queue.ChannelQueue =
		(XAsu_ChannelQueue *)XASU_SHARED_MEMORY_P0_CH_QUEUE;
	ClientInstancePtr->P1Queue.ChannelQueue =
		(XAsu_ChannelQueue *)XASU_SHARED_MEMORY_P1_CH_QUEUE;
	ClientInstancePtr->P0Queue.NextFreeIndex = 0U;
	ClientInstancePtr->P1Queue.NextFreeIndex = 0U;

	ClientInstancePtr->IsReady = XASU_CLIENT_READY;

	Status = XMailbox_SetCallBack(ClientInstancePtr->MailboxPtr, XMAILBOX_RECV_HANDLER,
				      XAsu_DoorBellToClient, ClientInstancePtr->MailboxPtr);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function updates the queue buffer status to notify the request is present
 *			generates a door bell to ASU.
 *
 * @param	QueueInfo	 Pointer to QueueInfo
 *
 * @return
 * 			- Returns XST_SUCCESS upon successful update
 *          - Otherwise, returns an error code.
 *
 *************************************************************************************************/
s32 XAsu_UpdateQueueBufferNSendIpi(XAsu_QueueInfo *QueueInfo)
{
	s32 Status = XST_FAILURE;
	XAsu_ChannelQueueBuf *QueueBufPtr;

	if (QueueInfo == NULL) {
		goto END;
	}
	/* Get Queue memory */
	QueueBufPtr = XAsu_GetChannelQueueBuf(QueueInfo);
	if (QueueBufPtr == NULL) {
		goto END;
	}

	QueueBufPtr->RespBufStatus = 0x0U;
	QueueBufPtr->ReqBufStatus = XASU_COMMAND_IS_PRESENT;

	if (QueueInfo->NextFreeIndex == (XASU_MAX_BUFFERS - 1U)) {
		/* TODO to point to zero index upon free */
		QueueInfo->NextFreeIndex = 0U;
	} else {
		QueueInfo->NextFreeIndex++;
	}

	/* Set IsCmdPresent to TRUE to indicate that the command is present in the Queue */
	QueueInfo->ChannelQueue->IsCmdPresent = TRUE;

	Status = XAsu_SendIpi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	while (!RecvDone);

	RecvDone = FALSE;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function returns the pointer of free XAsufw_ChannelQueueBuf of the requested
 *			priority queue
 *
 * @param	QueueInfo	 Pointer to QueueInfo
 *
 * @return
 * 			- Pointer to XAsufw_ChannelQueueBuf
 *          - Otherwise, returns NULL
 *
 *************************************************************************************************/
XAsu_ChannelQueueBuf *XAsu_GetChannelQueueBuf(XAsu_QueueInfo *QueueInfo)
{
	XAsu_ChannelQueueBuf *QueueBuf = NULL;

	if (QueueInfo == NULL) {
		goto END;
	}

	/* Check if Queue is full */
	if (XAsu_IsChannelQueueFull(QueueInfo) != TRUE) {
		QueueBuf =  &QueueInfo->ChannelQueue->ChannelQueueBufs[QueueInfo->NextFreeIndex];
	}

END:
	return QueueBuf;

}

/*************************************************************************************************/
/**
 * @brief   This function returns either requested queue is full or not.
 *
 * @param	QueuePriority	 Priority of the queue
 *
 * @return
 * 			- TRUE - if queue is full
 *          - Otherwise, returns FALSE
 *
 *************************************************************************************************/
static u32 XAsu_IsChannelQueueFull(XAsu_QueueInfo *QueueInfo)
{
	return (QueueInfo->NextFreeIndex <= (XASU_MAX_BUFFERS - 1U)) ? FALSE : TRUE;
}

/*************************************************************************************************/
/**
 * @brief   This function returns the pointer to the XAsu_QueueInfo structure of provided priority
 *
 * @param	QueuePriority	 Priority of the queue
 *
 * @return
 * 			- NULL if invalid input
 *          - Pointer to XAsu_QueueInfo
 *
 *************************************************************************************************/
XAsu_QueueInfo *XAsu_GetQueueInfo(u32 QueuePriority)
{
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();
	XAsu_QueueInfo *QueueInfo = NULL;

	if (QueuePriority == XASU_PRIORITY_HIGH) {
		QueueInfo = &ClientInstancePtr->P0Queue;
	} else if (QueuePriority == XASU_PRIORITY_LOW) {
		QueueInfo = &ClientInstancePtr->P1Queue;
	} else {
		QueueInfo = NULL;
	}

	return QueueInfo;
}

/*************************************************************************************************/
/**
 * @brief  This function sends an IPI request to ASU
 *
 * @return
 *	-	XST_SUCCESS - If the IPI is sent successfully
 *	-	XST_FAILURE - If there is a failure
 *
 *************************************************************************************************/
static s32 XAsu_SendIpi(void)
{
	s32 Status = XST_FAILURE;
	XAsu_Client *ClientInstancePtr = XAsu_GetClientInstance();

	if (ClientInstancePtr->IsReady != XASU_CLIENT_READY) {
		goto END;
	}

	Status = (s32)XMailbox_Send(ClientInstancePtr->MailboxPtr,
				    XASU_TARGET_IPI_INT_MASK, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function returns an instance pointer of ASU client interface.
 *
 *
 * @return
 * 			- It returns pointer to the XAsu_Client.
 *
 *************************************************************************************************/
static XAsu_Client *XAsu_GetClientInstance(void)
{
	static XAsu_Client ClientInstance = {0U};

	return &ClientInstance;
}

/****************************************************************************/
/**
 * @brief  This function polls for the response
 *
 * @return
 *  -   XST_SUCCESS - upon success
 *  -   XST_FAILURE - If there is a failure
 *
 ****************************************************************************/
static void XAsu_DoorBellToClient(void *CallBackRef)
{
	(void)CallBackRef;
	RecvDone = TRUE;
}

/*************************************************************************************************/
/**
 * @brief   This function returns status based on presence of ASUFW application.
 *
 *
 * @return
 *  -   XST_SUCCESS - upon success
 *  -   XST_FAILURE - If there is a failure
 *
 *************************************************************************************************/
static s32 XAsu_CheckAsufwPrsntBit(void)
{
	s32 Status = XST_FAILURE;
	s32 Timeout = 0U;

	for (Timeout = 0U; Timeout != XASU_ASUFW_BIT_CHECK_TIMEOUT_VALUE; Timeout++) {
		if ((Xil_In32(ASU_GLOBAL_GLOBAL_CNTRL) & ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
			== ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
				Status = XST_SUCCESS;
				goto END;
		}
		usleep(1U);
	}

END:
	return Status;
}
