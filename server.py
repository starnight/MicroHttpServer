#!/usr/bin/env python3

import socket
import select
import datetime

class Message:
	'''HTTP message class.'''
	def __init__(self):
		self.Header = []
		self.Body = None

def _HelloPage(req, res):
	'''Default Hello page which makes response message body.'''
	res.Body = "<html><body>許功蓋 Hello {}</body></html>".format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

class HTTPServer:
	def __init__(self, host="", port=8000):
		self.HOST = host
		self.PORT = port
		self._insocks = []
		self._outsocks = []
		# Create server socket.
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		# Set the socket could be reused directly after this application be closed.
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		# Set the socket non-blocking.
		self.sock.setblocking(0)

	def Start(self):
		self.sock.bind((self.HOST, self.PORT))

	def Listen(self, callback=_HelloPage):
		self.sock.listen(5)
		self._insocks.append(self.sock)
		while len(self._insocks) > 0:
			readable, writeable, exceptional = select.select(self._insocks, self._outsocks, self._insocks)
			for s in readable:
				if s is self.sock:
					self._Accept(s)
				else:
					self._Request(s, callback)

			for s in writeable:
				s.send("")
				self._outsocks.remove(s)
				s.close()

			for s in exceptional:
				self._insocks.remove(s)
				if s in self._outsocks:
					self._outsocks.remove(s)
				s.close()

	def _Accept(self, conn):
		conn, addr = self.sock.accept()
		print("{} {} connected".format(str(datetime.datetime.now()), addr))
		conn.setblocking(0)
		self._insocks.append(conn)

	def _Request(self, conn, callback):
		conn.recv(1024)
		request = Message()
		response = Message()
		self._ParseHeader(conn, request)
		self._ParseBody(conn, request)
		callback(request, response)
		self._BuildHeader(response)
		self._SendHeader(conn, response)
		self._SendBody(conn, response)

		self._insocks.remove(conn)
		if conn in self._outsocks:
			self._outsocks.remove(conn)

		conn.close()
		del request
		del response

	def _ParseHeader(self, conn, req):
		'''Get the request message header.'''
		print("\tParse header")

	def _ParseBody(self, conn, req):
		'''Get the request message body.'''
		print("\tParse body")

	def _BuildHeader(self, res):
		'''Build basement response message header.'''
		res.Header.insert(0, ["HTTP", "1.1"])
		if (len(res.Header) < 2) or (res.Header[1][0] == "Status"):
			res.Header.insert(1, ["Status", "200 OK"])
		res.Header.append(["content-type", "text/html; charset=UTF-8;"])
		res.Header.append(["content-length", len(str.encode(res.Body))])

	def _SendHeader(self, conn, res):
		'''Send the response message header.'''
		print("\tSend header")
		conn.send(str.encode("{}/{} ".format(res.Header[0][0], res.Header[0][1])))
		conn.send(str.encode("{}\r\n".format(res.Header[1][1])))
		for i in range(2, len(res.Header)):
			conn.send(str.encode("{}: {}\r\n".format(res.Header[i][0], res.Header[i][1])))
		conn.send(str.encode("\r\n"))

	def _SendBody(self, conn, res):
		'''Send the response message body.'''
		print("\tSend body")
		conn.send(str.encode(res.Body))

	def __del__(self):
		print("Close socket")
		self.sock.close()
		for s in self._insocks:
			s.close()
		for s in self._outsocks:
			s.close()

if __name__ == "__main__":
	server = HTTPServer(port=8000)
	print("Server is starting!!!")
	server.Start()
	print("Server is started!!!")
	server.Listen()
