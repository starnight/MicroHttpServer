#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "bits/socket.h"
#include "bits/mac_esp8266.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define MAX_SOCKETBUFLEN	2048

typedef struct _sock_struct {
	uint32_t peer_ip; /* Peer IP address. */
	uint16_t peer_port; /* Peer port. */
	uint16_t fd; /* File descriptor of the socket. */
	uint16_t slen; /* Length of the payload going to be sent to peer. */
	uint16_t rlen; /* Length of the payload going to receive from peer. */
	QueueHandle_t rxQueue; /* Buffer for receive from client. */
	uint8_t *sbuf; /* Points to the buffer going to be sent. */
	uint8_t state; /* Socket status. */
} _sock;

_sock clisock[MAX_CLIENT + 1];
#define svrsock	(clisock[MAX_CLIENT])

/* Set server socket's ID being last socket ID. */
#define SERVER_SOCKET_ID	MAX_CLIENT

/* Sockets pool queue handler. */
QueueHandle_t new_connects;

/* Socket and ID mapping macroes. */
#define ID2Sock(id)	((SOCKET)(id + SOCKET_BASE))
#define Sock2ID(s)	(s - SOCKET_BASE)

uint8_t _ESP8266_state;

/* ESP8266 UART channel usage mutex. */
SemaphoreHandle_t xUSART_Mutex = NULL;

uint8_t _ESP8266_Command;
SOCKET	_working_sock;

#define ESP8266_NONE_COMMAND	0
#define ESP8266_SEND_COMMAND	1
#define ESP8266_CLOSE_COMMAND	2

#define USART_RXBUFLEN	64

uint8_t USART_rBuf[USART_RXBUFLEN];
uint16_t USART_rIdx;
uint16_t USART_wIdx;

TaskHandle_t xCommandTask;

#define IsNumChar(c) (('0' <= c) && (c <= '9'))

/* Get the status of ESP8266. */
uint8_t GetESP8266State(void) {
	return _ESP8266_state;
}

/* Try to parse connected by a new client. */
void GetClientConnented(void) {
	uint16_t id;
	SOCKET s;

	char debug[80];

	if(sscanf(USART_rBuf, "%d,CONNECT\r\n", &id) > 0) {
		s = ID2Sock(id);
		/* Set initial state of the client sock. */
		clisock[id].fd = s;
		clisock[id].rlen = 0;
		clisock[id].slen = 0;
		xQueueReset(clisock[Sock2ID(s)].rxQueue);
		clisock[id].state = 0;
		_SET_BIT(clisock[id].state, SOCKET_USING);
		/* Notify os server socket connected. */
		if(xQueueSendToBack(new_connects, &s, 0) == pdPASS) {
			_SET_BIT(svrsock.state, SOCKET_READABLE);
		}
	}
}

/* Try to parse request from a client. */
void GetClientRequest(void) {
	uint16_t id;
	uint16_t len;
	uint16_t n;
	uint8_t c;

	char debug[80];

	if(sscanf(USART_rBuf, "+IPD,%d,%d:", &id, &len) == 2) {
		clisock[id].rlen = len;
		if(len > MAX_SOCKETBUFLEN)
			len = MAX_SOCKETBUFLEN;

		n = 0;
		while(n < len) {
			if(USART_Read(USART6, &c, 1, NON_BLOCKING) > 0) {
				/* Read 1 byte from ESP8266 UART channel and
				 * send it to related scoket. */
				if(xQueueSendToBack(clisock[id].rxQueue, &c, 0) == pdTRUE)
					n++;
			}
			else {
				/* Wait for read 1 byte from ESP8266 UART channel. */
				vTaskDelay(portTICK_PERIOD_MS);
			}
		}
		/* Notify client socket there is something to be read 
		 * after receive a frame. */
		_SET_BIT(clisock[id].state, SOCKET_READABLE);

		clisock[id].rlen -= n;
	}
}

/* Try to parse closed by a client. */
void GetClientClosed(void) {
	uint16_t id;
	SOCKET s;

	char debug[30];

	if(sscanf(USART_rBuf, "%d,CLOSED\r\n", &id) > 0) {
		/* Notify os server socket close. */
		s = ID2Sock(id);
		/* Clear state of the client sock. */
		xQueueReset(clisock[id].rxQueue);
		clisock[id].state = 0;
		snprintf(debug, 30, "\t\t\tFD: %d, ID: %d closed\r\n", s, id);
		USART_Printf(USART2, debug);
	}
}

/* Try to dispatch the request from ESP8266 UART channel. */
void GetESP8266Request(void) {
	ssize_t n;
	uint8_t c;
	char s[12];

	/* Have request header from ESP8266 UART channel. */
	USART_wIdx=0;
	do {
		if(USART_Read(USART6, &c, 1, NON_BLOCKING) > 0) {
			USART_rBuf[USART_wIdx] = c;
			USART_wIdx++;
		}
		else {
			vTaskDelay(50);
		}
	} while(((c != '\n') && (c != ':')) && (USART_wIdx < USART_RXBUFLEN-1));
	USART_rBuf[USART_wIdx] = '\0';

	/* Try to parse request header. */
	USART_rIdx = 0;
	if(USART_wIdx < 4) {
		/* Useless message. */
	}
	else if(strncmp(USART_rBuf, "WIFI GOT IP", 11) == 0) {
		/* Change ESP8266 UART channel state is ready for internet usage. */
		_ESP8266_state = ESP8266_LINKED;
	}
	else if(IsNumChar(USART_rBuf[0])) {
		/* Number of ID part. */
		for(;
			IsNumChar(USART_rBuf[USART_rIdx]) &&
				(USART_rIdx < (USART_wIdx - strlen(",CLOSED\r\n")));
			USART_rIdx++);
		if(USART_rBuf[USART_rIdx+2] == 'O') {
			/* Go parse new ID connected. */
			GetClientConnented();
		}
		else if(USART_rBuf[USART_rIdx+2] == 'L') {
			/* Go parse an ID closed. */
			GetClientClosed();
		}
	}
	else if(strncmp(USART_rBuf, "+IPD,", 5) == 0) {
		/* Go parse ID request. */
		GetClientRequest();
	}
	else if(strncmp(USART_rBuf, "WIFI DISCONNECT", 15) == 0) {
		/* Change ESP8266 UART channel state is not ready for internet usage. */
		_ESP8266_state = ESP8266_NOT_LINKED;
	}
	else if(strncmp(USART_rBuf, "WIFI CONNECTED", 14) == 0) {
		/* Ignore for now. */
	}
	else if(strncmp(USART_rBuf, "Ai-Thinker Technology Co.,Ltd.", 30) == 0) {
		/* Change ESP8266 UART channel acts as a WiFi NIC. */
		_ESP8266_state = ESP8266_INITIALIZED;
	}
}

/* ESP8266 UART channel request parsing task. */
void vESP8266RTask(void *__p) {
	/* Enable the pipe with ESP8266 UART channel. */
	USART_EnableRxPipe(USART6);

	while(1) {
		/* Try to take ESP8266 UART channel usage mutex. */
		if(xSemaphoreTake(xUSART_Mutex, 100) == pdTRUE) {
			while(USART_Readable(USART6)) {
				/* There is a request from ESP8266 UART channel. */
				GetESP8266Request();
			}
			/* Parse finished and releas ESP8266 UART channel usage mutex. */
			xSemaphoreGive(xUSART_Mutex);
		}
		/* Wait for ESP8266 UART channel usage mutex or new request from ESP8266
		 * UART channel. */
		vTaskDelay(10*portTICK_PERIOD_MS);
	}
}

void vSendSocketTask(void *);
void vCloseSocketTask(void *);

void vESP8266TTask(void *__p) {
	while(1) {
		/* Try to take ESP8266 UART channel usage mutex. */
		if(xSemaphoreTake(xUSART_Mutex, 100) == pdTRUE) {
			switch(_ESP8266_Command){
			case ESP8266_SEND_COMMAND:
				vSendSocketTask(&_working_sock);
				break;
			case ESP8266_CLOSE_COMMAND:
				vCloseSocketTask(&_working_sock);
				break;
			default: break;
			}
			/* Send command finished and releas ESP8266 UART channel usage
			 * mutex. */
			xSemaphoreGive(xUSART_Mutex);
			/* Send command finished and suspend send command task. */
			vTaskSuspend(NULL);
		}
		else {
			vTaskDelay(1000);
		}
	}
}

void InitESP8266(void) {
	uint16_t i;
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	/* Zero ESP8266 state. */
	_ESP8266_state = ESP8266_NONE;
	_ESP8266_Command = ESP8266_NONE_COMMAND;

	/* Zero client sockets' state. */
	for(i=0; i<MAX_CLIENT; i++) {
		clisock[i].state = 0;
		clisock[i].rxQueue = xQueueCreate(MAX_SOCKETBUFLEN, sizeof(uint8_t));
	}
	/* Zero server socket's state. */
	svrsock.state = 0;

	/* Create ESP8266 UART channel usage mutex. */
	xUSART_Mutex = xSemaphoreCreateMutex();
	/* Create the new socket client connection queue. */
	new_connects = xQueueCreate(MAX_CLIENT, sizeof(SOCKET));

	/* Start USART for ESP8266. */
	setup_usart();

	/* Create ESP8266 parsing request from USART RX task. */
	xReturned = xTaskCreate(vESP8266RTask,
				"ESP8266 RX",
				300,
				NULL,
				tskIDLE_PRIORITY + 1,
				&xHandle);
	if(xReturned != pdPASS)
		USART_Printf(USART2, "Create RX task failed\r\n");
	
	/* Create ESP8266 command task to USART TX. */
	xReturned = xTaskCreate(vESP8266TTask,
				"ESP8266 TX",
				300,
				NULL,
				tskIDLE_PRIORITY + 1,
				&xCommandTask);
	if(xReturned == pdPASS)
		vTaskSuspend(xCommandTask);
	else
		USART_Printf(USART2, "Create TX task failed\r\n");
}

int JoinAccessPoint(char *ap, char *pwd) {
	char connect_ap[52];
	char res[60];
	ssize_t l, n;

	char debug[60];

	/* Make sure there is no pending message of ESP8266 RX data. */
	while(USART_Readable(USART6))
		vTaskDelay(100);
	/* Block to take ESP8266 UART channel usage mutex. */
	while(xSemaphoreTake(xUSART_Mutex, 100) != pdTRUE)
		vTaskDelay(50);

	/* ESP8266 joins an AP. */
	snprintf(connect_ap, 50, "AT+CWJAP=\"%s\",\"%s\"\r\n", ap, pwd);
	USART_Send(USART6, connect_ap, strlen(connect_ap), NON_BLOCKING);
	l = strlen(connect_ap) + 1;
	n = 0;
	do {
		n += USART_Read(USART6, res, l-n, BLOCKING);
		if(n < l)
			vTaskDelay(1000);
	} while(n < l);
	res[n] = '\0';
	strncpy(connect_ap+(strlen(connect_ap)-1), "\r\n", 3);

	if(strncmp(res, connect_ap, strlen(connect_ap)) != 0) {
		/* Releas ESP8266 UART channel usage mutex. */
		xSemaphoreGive(xUSART_Mutex);

		snprintf(debug, 60, "Join %s AP failed\r\n", ap);
		USART_Printf(USART2, debug);
		return -1;
	}

	/* Releas ESP8266 UART channel usage mutex. */
	xSemaphoreGive(xUSART_Mutex);

	snprintf(debug, 60, "Joined %s AP\r\n", ap);
	USART_Printf(USART2, debug);

	return 0;
}

int HaveInterfaceIP(uint32_t *pip) {
	char get_ip[] = "AT+CIFSR\r\n";
	char res[40];
	uint8_t ip[4];
	ssize_t l, n;

	char debug[30];

	/* Make sure there is no pending message of ESP8266 RX data. */
	while(USART_Readable(USART6))
		vTaskDelay(100);

	/* Block to take ESP8266 UART channel usage mutex. */
	while(xSemaphoreTake(xUSART_Mutex, 100) != pdTRUE)
		vTaskDelay(50);

	/* Get ESP8266 station IP. */
	USART_Send(USART6, get_ip, strlen(get_ip), NON_BLOCKING);
	l = strlen(get_ip) + 1;
	n = 0;
	do {
		n += USART_Read(USART6, res, l-n, BLOCKING);
		if(n < l)
			vTaskDelay(1000);
	} while(n < l);
	res[n] = '\0';

	if(strncmp(res, "AT+CIFSR\r\r\n", 11) != 0) {
		/* Releas ESP8266 UART channel usage mutex. */
		xSemaphoreGive(xUSART_Mutex);
		return -1;
	}

	/* Read IPs and MACs. */
	for(l=0; l<4; l++) { // Need 4 '\n'
		n = -1;
		do {
			n++;
			while(USART_Read(USART6, &res[n], 1, BLOCKING) < 1) {
				vTaskDelay(100);
			}
		} while(res[n] != '\n');
		res[n + 1] = '\0';

		if(strncmp(res, "+CIFSR:STAIP,", 13) == 0) {
			sscanf(res, "+CIFSR:STAIP,\"%d.%d.%d.%d\"",
			       &ip[0], &ip[1], &ip[2], &ip[3]);
			*pip = ip[0] << 24 + ip[1] << 16 + ip[2] << 8 + ip[3];
		}
	}

	l = strlen("\r\nOK\r\n");
	n = 0;
	do {
		n += USART_Read(USART6, res, l-n, BLOCKING);
	} while(n < l);
	res[n] = '\0';

	/* Releas ESP8266 UART channel usage mutex. */
	xSemaphoreGive(xUSART_Mutex);

	if(strncmp(res, "\r\nOK\r\n", 6) == 0) {
		snprintf(debug, 30, "\tGet ip %d.%d.%d.%d ok!\r\n",
			 ip[0], ip[1], ip[2], ip[3]);
		USART_Printf(USART2, debug);
		return 0;
	}
	else {
		USART_Printf(USART2, "\tGet ip failed!\r\n");
		errno = EBADF;
		return -1;
	}
}

SOCKET HaveTcpServerSocket(void) {
	if(!_ISBIT_SET(svrsock.state, SOCKET_USING)) {
		/* First time to have server socket.
		 * There is only one server socket should be. */
		svrsock.fd = ID2Sock(SERVER_SOCKET_ID);
		svrsock.state = 0;
		_SET_BIT(svrsock.state, SOCKET_WRITING);
		_SET_BIT(svrsock.state, SOCKET_USING);
		return svrsock.fd;
	}
	else {
		/* No more server socket should be. */
		errno = ENFILE;
		return -1;
	}
}

int BindTcpSocket(uint16_t port) {
	char mul_con[] = "AT+CIPMUX=1\r\n";
	char as_server[30];
	char res[30];
	TaskHandle_t task;
	ssize_t l, n;

	char debug[44];

	if(new_connects == NULL)
		new_connects = xQueueCreate(MAX_CLIENT, sizeof(SOCKET));

	/* Make sure there is no pending message of ESP8266 RX data. */
	while(USART_Readable(USART6))
		vTaskDelay(100);

	/* Block to take ESP8266 UART channel usage mutex. */
	while(xSemaphoreTake(xUSART_Mutex, 0) != pdTRUE)
		vTaskDelay(50);

	/* Enable ESP8266 multiple connections. */
	USART_Send(USART6, mul_con, strlen(mul_con), NON_BLOCKING);
	l = strlen(mul_con) + strlen("\r\nOK\r\n") + 1;
	n = 0;
	do {
		n += USART_Read(USART6, res+n, l-n, BLOCKING);
	} while(n < l);
	res[n] = '\0';

	if(strncmp(res, "AT+CIPMUX=1\r\r\n\r\nOK\r\n", 20) != 0) {
		/* Releas ESP8266 UART channel usage mutex. */
		xSemaphoreGive(xUSART_Mutex);
		return -1;
	}

	/* Set ESP8266 as server and listening on designated port. */
	snprintf(as_server, 30, "AT+CIPSERVER=1,%d\r\n", port);
	USART_Send(USART6, as_server, strlen(as_server), NON_BLOCKING);
	l = strlen(as_server) + strlen("\r\nOK\r\n") + 1;
	n = 0;
	do {
		n += USART_Read(USART6, res+n, l-n, BLOCKING);
		if(n < l)
			vTaskDelay(20);
	} while(n < l);
	res[n] = '\0';

	/* Releas ESP8266 UART channel usage mutex. */
	xSemaphoreGive(xUSART_Mutex);

	snprintf(as_server, 30, "AT+CIPSERVER=1,%d\r\r\n\r\nOK\r\n", port);
	if(strncmp(res, as_server, strlen(as_server)) == 0) {
		USART_Printf(USART2, "\tBind socket ok!\r\n");
		return 0;
	}
	else {
		USART_Printf(USART2, "\tBind socket failed!\r\n");
		errno = EBADF;
		return -1;
	}
}

SOCKET AcceptTcpSocket(void) {
	SOCKET s;


	if(xQueueReceive(new_connects, &s, 0) == pdTRUE) {
		/* Have a new connected socket. */
		/* Check is there still new clients from server socket. */
		if(uxQueueMessagesWaiting(new_connects) > 0)
			_SET_BIT(svrsock.state, SOCKET_READABLE);
		else
			_CLR_BIT(svrsock.state, SOCKET_READABLE);

		return s;
	}
	else {
		/* No new connected socket. */
		errno = ENODATA;
		return -1;
	}
}

ssize_t RecvSocket(SOCKET s, void *buf, size_t len, int f) {
	uint16_t id = Sock2ID(s);
	uint16_t i;
	uint8_t *pBuf;
	uint8_t c;

	pBuf = buf;

	for(i=0; i<len; i++) {
		if(xQueueReceive(clisock[id].rxQueue, &c, 0))
			pBuf[i] = c;
		else
			break;
	}
	/* Check there are still more bytes to be read. */
	if(uxQueueMessagesWaiting(clisock[id].rxQueue) > 0)
		_SET_BIT(clisock[id].state, SOCKET_READABLE);
	else
		_CLR_BIT(clisock[id].state, SOCKET_READABLE);

	return i;
}

void vSendSocketTask(void *__p) {
	SOCKET *ps;
	uint16_t id;
	uint16_t len;
	char send_header[30]; /* "AT+CIPSEND=id,len\r\n" */
	char res[32];
	ssize_t l, n;

#define MAX_SOCKETSENDBUFLEN	2048

	ps = (SOCKET *)__p;
	id = Sock2ID(*ps);

	while(clisock[id].slen > 0) {
		/* Split going to send packet into frame size. */
		if(clisock[id].slen > MAX_SOCKETSENDBUFLEN)
			len = MAX_SOCKETSENDBUFLEN;
		else
			len = clisock[id].slen;

		/* Have send frame header which is send command. */
		snprintf(send_header, 30, "AT+CIPSEND=%d,%d\r\n", id, len);
		/* Send socket send command to ESP8266. */
		USART_Send(USART6, send_header, strlen(send_header), BLOCKING);
		/* Have ESP8266 response message. */
		l = strlen(send_header) + strlen("\r\nOK\r\n> ") + 1;
		n = 0;
		do {
			n += USART_Read(USART6, res+n, l-n, BLOCKING);
			if(n < l)
				vTaskDelay(1000);
		} while(n < l);
		res[n] = '\0';

		snprintf(send_header, 30, "AT+CIPSEND=%d,%d\r\r\n\r\nOK\r\n> ",
			 id, len);
		if(strncmp(res, send_header, strlen(send_header)) != 0)
			break;

		/* Send socket payload to ESP8266. */
		USART_Send(USART6, clisock[id].sbuf, len, NON_BLOCKING);
		/* Have ESP8266 response message. */
		for(n=0; (n < 32); n++) {
			while(USART_Read(USART6, res+n, 1, BLOCKING) <= 0) {
				vTaskDelay(100);
			}
			if((res[n] == '\n') && (strncmp(&res[n-3], "OK\r\n", 4) == 0)) {
				n++;
				break;
			}
		}
		res[n] = '\0';

		if(sscanf(res, "\r\nRecv %d bytes\r\n\r\nSEND OK\r\n", &len) > 0) {
		}

		clisock[id].slen -= len;
		clisock[id].sbuf += len;
	}
	/* Finish writing and clear the socket is not writing now. */
	_CLR_BIT(clisock[id].state, SOCKET_WRITING);
	
	/* Can send another AT command to ESP8266 UART channel. */
	_ESP8266_Command = ESP8266_NONE_COMMAND;
}

ssize_t SendSocket(SOCKET s, void *buf, size_t len, int f) {
	uint16_t id = Sock2ID(s);
	ssize_t r;

	/* Check there are data to be sent and the socket is ready to send. */
	if((len > 0) && IsSocketReady2Write(s)) {
		/* Set the socket is busy in writing now. */
		_SET_BIT(clisock[id].state, SOCKET_WRITING);

		/* Make sure there is no pending message of ESP8266 RX data. */
		while(USART_Readable(USART6)
		      || (_ESP8266_Command != ESP8266_NONE_COMMAND)) {
			vTaskDelay(100);
		}

		/* The socket is writeable now. */
		/* Ask to send send socket command to ESP8266 UART channel. */
		clisock[id].sbuf = buf;
		clisock[id].slen = len;
		_working_sock = s;
		_ESP8266_Command = ESP8266_SEND_COMMAND;
		vTaskResume(xCommandTask);

		errno = EAGAIN;
		r = -1;
	}
	else {
		r = 0;
	}

	return r;
}

void vCloseSocketTask(void *__p) {
	SOCKET *ps;
	uint16_t id;
	char sd_sock[18]; /* Going to be "AT+CIPCLOSE=id\r\n" */
	char res[24];
	uint8_t n;
	uint8_t num_spliter;

	ps = (SOCKET *)__p;
	id = Sock2ID(*ps);

	/* Have close socket send command. */
	snprintf(sd_sock, sizeof(sd_sock), "AT+CIPCLOSE=%d\r\n", id);
	/* Send close socket command to ESP8266. */
	USART_Send(USART6, sd_sock, strlen(sd_sock), NON_BLOCKING);
	/* Have ESP8266 response message. */
	num_spliter = 0;
	for(n=0; n<24; n++) {
		while(USART_Read(USART6, res+n, 1, BLOCKING) <= 0)
			vTaskDelay(100);

		if(res[n] == '\n')
			num_spliter++;
		if(num_spliter == 3) {
			n++;
			break;
		}
	}
	res[n] = 0;

	if(sscanf(res, "%d,CLOSED\r\r\n\r\nOK\r\n", &id) > 0)
		clisock[id].state = 0;


	/* Can send another AT command to ESP8266 UART channel. */
	_ESP8266_Command = ESP8266_NONE_COMMAND;
}

int ShutdownSocket(SOCKET s, int how) {
	uint16_t id = Sock2ID(s);

	if(_ISBIT_SET(clisock[id].state, SOCKET_USING)) {
		/* Make sure there is no pending message of ESP8266 RX data. */
		while(USART_Readable(USART6)
		      || (_ESP8266_Command != ESP8266_NONE_COMMAND)) {
			vTaskDelay(1000);
		}

		/* Ask to send close socket command to ESP8266 UART channel. */
		_working_sock = s;
		_ESP8266_Command = ESP8266_CLOSE_COMMAND;
		vTaskResume(xCommandTask);
	}
	else {
		USART_Printf(USART2, "\t\tClose socket failed\r\n");
	}

	return 0;
}

int IsSocketReady2Read(SOCKET s) {
	uint16_t id = Sock2ID(s);
	uint8_t mask = (1 << SOCKET_USING) | (1 << SOCKET_READABLE);
	uint8_t f;

	f = 0;

	/* Check the socket's state. */
	if(id <= MAX_CLIENT)
		if((clisock[id].state & mask) == mask)
			f = 1;
	
	return f;
}

int IsSocketReady2Write(SOCKET s) {
	uint16_t id = Sock2ID(s);
	uint8_t mask = (1 << SOCKET_USING) | (1 << SOCKET_WRITING);
	uint8_t f;

	f = 0;

	/* Check the socket's state. */
	if(id < MAX_CLIENT)
		if((clisock[id].state & mask) == (1 << SOCKET_USING))
			f = 1;

	return f;
}
