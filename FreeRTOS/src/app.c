#include <string.h>
#include <stdlib.h>
#include "gpio.h"
#include "app.h"

void HelloPage(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0;
	unsigned int j;
	char *p;
	char header1[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	char header2[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";
	char body1[] = "<html><body>Hello!<br>許功蓋<br>" \
		       "<form method='POST' action='led'>" \
		       "GREEN LED=<input type='text' name='GREEN' value='0' /><br/>" \
		       "ORANGE LED=<input type='text' name='ORANGE' value='0' /><br/>" \
		       "RED LED=<input type='text' name='RED' value='0' /><br/>" \
		       "BLUE LED=<input type='text' name='BLUE' value='0' /><br />" \
		       "<input type='submit' />" \
		       "</form>";
	char body2[] = "</body></html>";

	/* Build header. */
	p = (char *)res->_buf;
	n = strlen(header1);
	memcpy(p, header1, n);
	p += n;
	i += n;

	n = strlen(header2);
	memcpy(p, header2, n);
	p += n;
	i += n;

	/* Build body. */
	n = strlen(body1);
	memcpy(p, body1, n);
	p += n;
	i += n;

	/* Echo request header into body. */
	n = strlen((char *)req->_buf);
	memcpy(p, req->_buf, n);
	p += n;
	i += n;

	n = strlen("<br>");
	memcpy(p, "<br>", n);
	p += n;
	i += n;

	n = strlen(req->Header.URI);
	memcpy(p, req->Header.URI, n);
	p += n;
	i += n;

	n = strlen("<br>");
	memcpy(p, "<br>", n);
	p += n;
	i += n;

	n = strlen(req->Header.Version);
	memcpy(p, req->Header.Version, n);
	p += n;
	i += n;

	for(j=0; j<req->Header.Amount; j++) {
		n = strlen("<br>");
		memcpy(p, "<br>", n);
		p += n;
		i += n;

		n = strlen(req->Header.Fields[j].key);
		memcpy(p, req->Header.Fields[j].key, n);
		p += n;
		i += n;

		p[0] = ':'; p[1] = ' ';
		p += 2;
		i += 2;

		n = strlen(req->Header.Fields[j].value);
		memcpy(p, req->Header.Fields[j].value, n);
		p += n;
		i += n;
	}

	n = strlen(body2);
	memcpy(p, body2, n);
	i += n;

	res->_index = i;
}

char *itoa(int n, char *s, int b) {
	char digits[] = "0123456789ABCDEF";
	uint8_t i = 0;
	int shift;
	char *p;

	p = s;

	/* Deal the sign. */
	if(n < 0) {
		s[0] = '-';
		n = -n;
		p += 1;
	}

	/* Convert integer to characters. */
	shift = n;
	if(shift == 0)
		i = 1;
	else
		for(; shift > 0; i++)
			shift /= b;

	p[i] = '\0';

	do {
		i--;
		p[i] = digits[n % b];
	} while((n /= b) > 0);

	return s;
}

int fibnacci(int l) {
	int sum = 0;
	int ppre = 0, pre = 1;

	if(l == 2)
		sum = 1;
	else {
		for(l -= 2; l > 0; l--) {
			sum = ppre + pre;
			ppre = pre;
			pre = sum;
		}
	}

	return sum;
}

void Fib(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0;
	char *p;
	char header1[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	char header2[] = "Content-Type: text/text; charset=UTF-8\r\n\r\n";
	int l;
	char *str;

	/* Build header. */
	p = (char *)res->_buf;
	n = strlen(header1);
	memcpy(p, header1, n);
	p += n;
	i += n;

	n = strlen(header2);
	memcpy(p, header2, n);
	p += n;
	i += n;

	/* Build body. */
	str = strstr((char *)req->Body, "Level=");
	if(str != NULL) {
		l = atoi(str + 6);
		n = strlen(itoa(fibnacci(l), p, 10));
		i += n;
	}
	else {
		memcpy(p, "Wrong", 6);
		i += 6;
	}

	res->_index = i;
}

#define NUM_LEDS	4

void LED(HTTPReqMessage *req, HTTPResMessage *res) {
	char *LED_str[NUM_LEDS] = {"GREEN=", "ORANGE=", "RED=", "BLUE="};
	uint32_t LEDs[NUM_LEDS] = {GREEN, ORANGE, RED, BLUE};
	char *str;
	int i;
	int8_t state, c;

	/* Go through the LEDs. */
	c = 0;
	for(i=0; i<NUM_LEDS; i++) {
		/* Find the LED strings. */
		str = strstr((char *)req->Body, LED_str[i]);

		if(str == NULL) {
			/* Not found the corresponding LED color string. */
			continue;
		}

		/* Found the corresponding LED color string. */
		c += 1;
		/* Get the config state. */
		state = atoi(str + strlen(LED_str[i]));
		/* Set the state of the corresponding LED. */
		if(state == 1)
			GPIO_SetBits(LEDS_GPIO_PORT, LEDs[i]);
		else if(state == 0)
			GPIO_ResetBits(LEDS_GPIO_PORT, LEDs[i]);
	}

	/* Re-render the Hello page. */
	HelloPage(req, res);
}
