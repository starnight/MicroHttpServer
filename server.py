#!/usr/bin/env python3

import socket
import select
import datetime

MAX_HEADER_SIZE = 2048

class HTTPError(Exception):
	'''Define an HTTP error exception.'''
	def __init__(self, message, error=1):
		super(HTTPError, self).__init__(message)
		self.error = error

class Message:
	'''HTTP message class.'''
	def __init__(self):
		self.Header = []
		self.Body = None
		self._Buf = None

def _HelloPage(req, res):
	'''Default Hello page which makes response message body.'''
	res.Body = "<html><body>許功蓋 Hello {}".format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
	for h in req.Header:
		res.Body += "<br>{}:{}".format(h[0], h[1])
	res.Body += "</body></html>"

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
		request = Message()
		response = Message()
		try:
			self._ParseHeader(conn, request)
			self._ParseBody(conn, request)
			callback(request, response)
			self._BuildHeader(response)
			self._SendHeader(conn, response)
			self._SendBody(conn, response)
		except HTTPError as e:
			print("\t{}".format(e))
		finally:
			self._insocks.remove(conn)
			if conn in self._outsocks:
				self._outsocks.remove(conn)

			conn.close()
		del request
		del response

	def _ParseHeader(self, conn, req):
		'''Get the request message header.'''
		print("\tParse header")

		# Socket read into request message buffer.
		self._Buf = conn.recv(MAX_HEADER_SIZE)

		# Build the request line.
		# Get method
		s = 0
		i = 3
		while self._Buf[i] != ord(" "):
			i += 1
		req.Header.append(["Method", self._Buf[s:i].decode("ASCII")])

		# Get URI
		i += 1
		s = i
		while self._Buf[i] != ord(" "):
			i += 1
		req.Header.append(["URI", self._Buf[s:i].decode("ASCII")])

		# Get HTTP version
		i += 1
		s = i
		i += 10
		if self._Buf[s:i].decode("ASCII") == "HTTP/1.1\r\n":
			req.Header.append(["HTTP", "1.1"])
		else:
			raise HTTPError("Client is not HTTP/1.1 protocal")

		# Build the request header feilds.
		s = i
		i += 2
		while self._Buf[s:i].decode("ASCII") != "\r\n":
			i += 1
			while self._Buf[i-2:i].decode("ASCII") != "\r\n":
				i += 1
			hf = self._Buf[s:i-2]

			j = 2
			while hf[j-2:j].decode("ASCII") != ": ":
				j += 1
			req.Header.append([hf[0:j-2].decode("ASCII"), hf[j:].decode("ASCII")])

			s = i
			i += 2

		return 0

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
