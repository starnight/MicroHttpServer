#include <ctype.h>
#include <string.h>
#include "usart.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Define the structure of stream going to be written out. */
typedef struct _STREAM {
	uint8_t *pBuf;
	size_t BufLen;
} STREAM;

/* Define the USART RX Queue for buffering. */

/* USART RX pipe enable flag. */
uint8_t rxPipeState;

typedef struct _rx_Queue {
	uint16_t wIdx;
	uint16_t rIdx;
	uint16_t rLen;
	uint8_t eSize;
	uint8_t ovr;
	uint8_t *buf;
} RX_QUEUE;

RX_QUEUE __rxQueue;
uint8_t __rxBuf[RX_QUEUELEN];
RX_QUEUE *rxQueue;

/* Global varables to hold the state of the streams going to be send. */
/* USART streams pool. */
STREAM usart_stream[MAX_USART_STREAM];
/* Index of current working stream. */
uint8_t usart_stream_idx;

/* Clear USART stream macro. */
#define ClearStream(stream) {(stream)->pBuf = NULL; (stream)->BufLen = 0;}

/* Set state USART RX pipe enable flag. */
void USART_SetRxPipe(USART_TypeDef *USARTx, int f) {
	if(USARTx == USART6)
		rxPipeState = f;
}

/* Push an element into the queue. */
int PushQueue(RX_QUEUE *__q, void *__pe) {
	uint8_t *b;
	uint8_t i;
	uint8_t f;

	b = __pe;
	f = 0;

	if((__q->ovr == 0) && (__q->wIdx < __q->rLen)) {
		f = 1;
	}
	else if((__q->ovr == 0) && (__q->rLen <= __q->wIdx) && (0 < __q->rIdx)) {
		f = 1;
		__q->ovr = 1;
		__q->wIdx = 0;
	}
	else if((__q->ovr == 1) && (__q->wIdx < __q->rIdx)) {
		f = 1;
	}

	if(f) {
		/* Zero copy an element to buffer according the size of each element. */
		for(i=0; i<__q->eSize; i++)
			*(__q->buf + __q->eSize*__q->wIdx + i) = b[i];
		/* Increment wIdx. */
		__q->wIdx += 1;
	}

	return f;
}

/* Pop and receive an element from the queue. */
int PopQueue(RX_QUEUE *__q, void *__pe) {
	uint8_t *b;
	uint8_t i;
	uint8_t f;

	b = __pe;
	f = 0;

	if((__q->ovr == 0) && (__q->rIdx < __q->wIdx)) {
		f = 1;
	}
	else if((__q->ovr == 1) && (__q->rLen <= __q->rIdx) && (0 < __q->wIdx)) {
		f = 1;
		__q->ovr = 0;
		__q->rIdx = 0;
	}
	else if((__q->ovr == 1) && (__q->rIdx < __q->rLen)) {
		f = 1;
	}

	if(f) {
		/* Zero copy an element from buffer according the size of each
		 * element. */
		for(i=0; i<__q->eSize; i++)
			b[i] = *(__q->buf + __q->eSize*__q->rIdx + i);
		/* Increment rIdx. */
		__q->rIdx += 1;
	}

	return f;
}

/* Get how many element in the queue. */
int QueueMessagesWaiting(RX_QUEUE *__q) {
	if(__q->ovr)
		return (__q->rLen - __q->rIdx + __q->wIdx);
	else
		return (__q->wIdx - __q->rIdx);
}

/* Clear the elements in the queue. */
void ResetQueue(RX_QUEUE *__q) {
	__q->wIdx = 0;
	__q->rIdx = 0;
	__q->ovr = 0;
}

/* Have an empty queue with designated number of elements
 * with designated size of each element. */
int CreateQueue(RX_QUEUE *__q, void *__pb, uint16_t __len, uint8_t __esize) {
	ResetQueue(__q);
	__q->rLen = __len;
	__q->eSize = __esize;
	__q->buf = __pb;

	return 1;
}

/* Initialize the USART6. */
void setup_usart(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIOC peripheral clock. */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	/* Make PC6, PC7 as alternative function of USART6. */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	/* Initialize PC6, PC7.  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Enable the USART6 peripheral clock. */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	/* Initialize USART6 with
	 * 115200 buad rate,
	 * 8 data bits,
	 * 1 stop bit,
	 * no parity check,
	 * none flow control.
	 */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART6, &USART_InitStructure);

	/* Enable USART6. */
	USART_Cmd(USART6, ENABLE);

	/* Initial the USART TX streams. */
	for(usart_stream_idx = MAX_USART_STREAM;
		usart_stream_idx > 0;
		usart_stream_idx--)
		ClearStream(usart_stream + usart_stream_idx - 1);
	/* Initial the RX Queue. */
	rxQueue = &__rxQueue;
	CreateQueue(rxQueue, __rxBuf, RX_QUEUELEN, sizeof(uint8_t));
	if(rxQueue != NULL)
		USART_Printf(USART2, "RX pipe created.\r\n");
	else
		USART_Printf(USART2, "RX pipe created failed.\r\n");
	/* Disable RX pipe first. */
	USART_DisableRxPipe(USART6);

	/* Enable USART6 RX interrupt. */
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
	/* Enable USART6 in NVIC vector. */
	NVIC_EnableIRQ(USART6_IRQn);
}

/* Initialize the USART2. */
void setup_usart2(void) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIOA peripheral clock. */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Make PA2, PA3 as alternative function of USART2. */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	/* Initialize PA2, PA3.  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable the USART2 peripheral clock. */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Initialize USART2 with
	 * 115200 buad rate,
	 * 8 data bits,
	 * 1 stop bit,
	 * no parity check,
	 * none flow control.
	 */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	/* Enable USART2. */
	USART_Cmd(USART2, ENABLE);
}

/* Read bytes array with designated length from RX Queue. */
ssize_t USART_Read(USART_TypeDef *USARTx, void *buf, ssize_t l, uint8_t flags) {
	ssize_t i = -1;
	uint8_t *b;

	b = buf;

	if(USARTx == USART6) {
		for(i=0; i<l; i++) {
			if(!PopQueue(rxQueue, &(b[i]))){ 
				break;
			}
		}
	}

	return i;
}

/* Check USART RX buffer is readable. */
int USART_Readable(USART_TypeDef *USARTx) {
	if(USARTx == USART6)
		return QueueMessagesWaiting(rxQueue);
	else
		return 0;
}

/* Send bytes array with designated length through USART. */
ssize_t USART_Send(USART_TypeDef *USARTx, void *buf, ssize_t l, uint8_t flags) {
	ssize_t i = 0;
	uint8_t idx;
	uint8_t *pBuf;

	pBuf = buf;

	/* Send with blocking mode. */
	if(flags == BLOCKING) {
		for(i=0; i<l; i++) {
			while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
			USART_SendByte(USARTx, (uint16_t)(pBuf[i]));
		}
	}
	/* Send with non-blocking mode. */
	else if(flags == NON_BLOCKING) {
		for(i=0; i<MAX_USART_STREAM; i++) {
			idx = (i + usart_stream_idx) % MAX_USART_STREAM;
			if(usart_stream[idx].pBuf == NULL) {
				usart_stream[idx].pBuf = pBuf;
				usart_stream[idx].BufLen = l;
				/* Enable USART6 TX interrupt. */
				USART_ITConfig(USART6, USART_IT_TXE, ENABLE);
				break;
			}
		}
		if(i >= MAX_USART_STREAM)
			i = -1;
	}

	return i;
}

/* Print the string through USART with blocking. */
void USART_Printf(USART_TypeDef* USARTx, char *str) {
	USART_Send(USARTx, str, strlen(str), BLOCKING);
}

/* USART6 IRQ handler. */
void USART6_IRQHandler(void) {
	uint8_t i;
	uint8_t rxdata;
	BaseType_t xHigherPriTaskWoken;

	xHigherPriTaskWoken = pdFALSE;

	/* USART6 RX interrupt. */
	if(USART6->SR & USART_SR_RXNE) {
		/* Push data into RX Queue. */
		rxdata = USART_ReadByte(USART6);
#ifdef MIRROR_USART6
		USART_SendByte(USART2, rxdata);
#endif
		if(rxPipeState > 0)
			PushQueue(rxQueue, &rxdata);
	}

	/* USART6 TX interrupt. */
	if(USART6->SR & USART_SR_TXE) {
		if(usart_stream[usart_stream_idx].BufLen > 0) {
			USART_SendByte(USART6, *usart_stream[usart_stream_idx].pBuf);
			usart_stream[usart_stream_idx].pBuf++;
			usart_stream[usart_stream_idx].BufLen--;
		}
		else {
			/* Current USART streaming is finished. */
			/* Release and clear current USART stream. */
			ClearStream(usart_stream + usart_stream_idx);
			/* Try to stream next USART stream which should be stream. */
			usart_stream_idx++;
			for(i=0; i<MAX_USART_STREAM; i++) {
				usart_stream_idx = (usart_stream_idx + i) % MAX_USART_STREAM;
				if(usart_stream[usart_stream_idx].pBuf != NULL)
					break;
			}
			/* Disable USART6 TX interrupt after all streams are finished. */
			if(i >= MAX_USART_STREAM)
				USART_ITConfig(USART6, USART_IT_TXE, DISABLE);
		}
	}
}
