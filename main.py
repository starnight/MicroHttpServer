#!/usr/bin/env python3

from server import HTTPServer
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

if __name__ == "__main__":
	print("Server is starting!!!")
	server = HTTPServer(port=8000)
	print("Server is started!!!")
	server.Listen(WellcomePage)
