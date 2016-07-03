# This file defines the server application functions (SAFs).

from datetime import datetime

def WellcomePage(req, res):
	'''Default wellcome page which makes response message.'''
	# Build HTTP message body
	res.Body = "<html><body>Hello!<br>"
	res.Body += "It's {} for now.".format(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
	res.Body += "</body></html>"

	# Build HTTP message header
	res.Header.append(["Status", "200 OK"])
	res.Header.append(["Content-Type", "text/html; charset=UTF-8;"])	

def fib(level):
	'''Fibnacci series.'''
	sum = 0
	ppre = 0
	pre = 1

	if level == 2:
		sum = 1
	elif level > 1:
		level -= 2
		while level > 0:
			sum = ppre + pre
			ppre = pre
			pre = sum
			level -= 1

	return sum

def atoi(s):
	num = 0;
	for c in s:
		if (c <= '9') and (c >= '0'):
			num = num * 10 + int(c)
		else:
			break

	return num

def Fib(req, res):
	'''Fibnacci Web API.'''
	# Build HTTP message header
	res.Header.append(["Status", "200 OK"])
	res.Header.append(["Content-Type", "text/text; charset=UTF-8;"])

	req.Body = req.Body.decode("ASCII")
	# Build HTTP message body
	level = atoi(req.Body[req.Body.find("Level=")+6:])
	res.Body = str(fib(level))
