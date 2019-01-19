#!/usr/bin/env python3

import socket
import select
import datetime

MAX_HEADER_SIZE = 2048
MAX_BODY_SIZE = 4096
MHS_PORT = 8000
MAX_HTTP_CLIENT = 5
HTTP_SERVER = "Micro PyHTTP Server"

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
		self._index = 0

def _HelloPage(req, res):
	'''Default Hello page which makes response message.'''
	# Build HTTP message body
	res.Body = "<html><body>許功蓋 Hello {}".format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
	for h in req.Header:
		res.Body += "<br>{}:{}".format(h[0], h[1])
	res.Body += "<br><br>{}".format(req.Body)
	res.Body += "</body></html>"

	# Build HTTP message header
	res.Header.append(["Status", "200 OK"])
	res.Header.append(["Content-Type", "text/html; charset=UTF-8;"])

class HTTPServer:
	global MHS_PORT
	global MAX_HTTP_CLIENT

	def __init__(self, host="", port=MHS_PORT):
		self.HOST = host
		self.PORT = port
		self.MAX_CLIENT = MAX_HTTP_CLIENT
		self._insocks = []
		self._outsocks = []
		# Create server socket.
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		# Set the socket could be reused directly after this application be closed.
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		# Set the socket non-blocking.
		self.sock.setblocking(0)
		self.sock.bind((self.HOST, self.PORT))

		# Start server socket listening.
		self.sock.listen(self.MAX_CLIENT)
		self._insocks.append(self.sock)

	def Run(self, callback):
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

	def RunLoop(self, callback):
		while True:
			self.Run(callback)

	def _Accept(self, conn):
		conn, addr = self.sock.accept()
		print("{} {} connected".format(str(datetime.datetime.now()), addr))
		#conn.setblocking(0)
		self._insocks.append(conn)

	def _Request(self, conn, callback):
		request = Message()
		response = Message()
		try:
			self._ParseHeader(conn, request)
			self._GetBody(conn, request)
			callback(request, response)
			self._BuildHeader(response)
			self._SendHeader(conn, response)
			self._SendBody(conn, response)
		except HTTPError as e:
			print("\t{}".format(e))
		except Exception as e:
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
		req._Buf = conn.recv(4)

		# Build the request line.
		# Get method
		s = 0
		i = 3
		while True:
			if req._Buf[i] != ord(" "):
				req._Buf += conn.recv(1)
				i += 1
			else:
				break
		req.Header.append(["Method", req._Buf[s:i].decode("ASCII")])

		# Get URI
		s = i + 1
		while True:
			req._Buf += conn.recv(1)
			i += 1
			if req._Buf[i] == ord(" "):
				break
		req.Header.append(["URI", req._Buf[s:i].decode("ASCII")])

		# Get HTTP version
		s = i + 1
		req._Buf += conn.recv(10)
		i += 10
		if req._Buf[s:i+1].decode("ASCII") == "HTTP/1.1\r\n":
			req.Header.append(["HTTP", "1.1"])
		else:
			raise HTTPError("Client is not HTTP/1.1 protocal.")

		# Build the request header feilds.
		s = i + 1
		req._Buf += conn.recv(2)
		i += 3
		while req._Buf[i-2:i].decode("ASCII") != "\r\n":
			while req._Buf[i-2:i].decode("ASCII") != "\r\n":
				req._Buf += conn.recv(1)
				i += 1
			hf = req._Buf[s:i-2]

			j = 2
			while hf[j-2:j].decode("ASCII") != ": ":
				j += 1
			req.Header.append([hf[0:j-2].decode("ASCII"), hf[j:].decode("ASCII")])

			s = i
			req._Buf += conn.recv(2)
			i += 2

		req._index = i

	def _GetBody(self, conn, req):
		'''Get the request message body.'''
		print("\tParse body")
		cl = 0
		i = 0
		if req.Header[0][1] == "POST":
			for fv in req.Header:
				if fv[0].lower() == "content-length":
					cl = int(fv[1])
					break

			if cl > MAX_BODY_SIZE:
				cl = MAX_BODY_SIZE
			buf = b""
			while i < cl:
				# Read more and concat if content length is bigger then read.
				buf += conn.recv(cl)
				i = len(buf)
			req._Buf += buf

		req.Body = req._Buf[req._index:]
		req._index += i

	def _BuildHeader(self, res):
		'''Build basement response message header.'''
		res.Header.insert(0, ["HTTP", "1.1"])
		if (len(res.Header) < 2) or (res.Header[1][0] != "Status"):
			res.Header.insert(1, ["Status", "200 OK"])

		checked = 0
		for fv in res.Header:
			if fv[0] == "Content-Type":
				checked = 1
				break
		if (checked != 1) and (res.Body is not None):
			res.Header.append(["Content-Type", "text/html; charset=UTF-8;"])

		if res.Body is not None:
			res.Header.append(["content-length", len(str.encode(res.Body))])
		else:
			res.Header.append(["content-length", 0])

		checked = 0
		for fv in res.Header:
			if fv[0] == "X-Powered-By":
				checked = 1
				break
		if checked != 1:
			res.Header.append(["X-Powered-By", HTTP_SERVER])

		res.Header.append(["Date", datetime.datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")])

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
		if res.Body is not None:
			buf = str.encode(res.Body)
			size = len(buf)
			totalsent = 0
			while totalsent < size:
				sent = conn.send(buf[totalsent:])
				if sent == 0:
					raise HTTPError("Send connection broken")
				totalsent += sent

	def __del__(self):
		print("Close socket")
		self.sock.close()
		for s in self._insocks:
			s.close()
		for s in self._outsocks:
			s.close()

if __name__ == "__main__":
	print("Server is starting!!!")
	server = HTTPServer(port=8000)
	print("Server is started!!!")
	while True:
		server.Run(_HelloPage)
