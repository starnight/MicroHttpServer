#!/usr/bin/env python3

import socket
import datetime

class Message:
	Header = [["HTTP", "1.1"], ["Status", "200 OK"]]
	Body = None

class HTTPServer:
	def __init__(self, host="", port=8000):
		self.HOST = host
		self.PORT = port
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	def Start(self):
		self.sock.bind((self.HOST, self.PORT))

	def Listen(self):
		self.sock.listen(5)
		while True:
			conn, addr = self.sock.accept()
			print("{} {} connected".format(str(datetime.datetime.now()), addr))
			conn.recv(1024)
			response = Message()
			response.Body = "<html><body>許功蓋 Hello {}</body></html>".format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
			self._BuildHeader(response)
			self._SendHeader(conn, response)
			self._SendBody(conn, response)
			conn.close()

	def _ParseHeader(self, conn, req):
		print("\tParse header")

	def _ParseBody(self, conn, req):
		print("\tParse body")

	def _BuildHeader(self, res):
		res.Header.append(["content-type", "text/html; charset=UTF-8;"])
		res.Header.append(["content-length", len(str.encode(res.Body))])

	def _SendHeader(self, conn, res):
		print("\tSend header")
		conn.send(str.encode("{}/{} \r\n".format(res.Header[0][0], res.Header[0][1])))
		conn.send(str.encode("{}\r\n".format(res.Header[1][1])))
		for i in range(2, len(res.Header)):
			conn.send(str.encode("{}: {}\r\n".format(res.Header[i][0], res.Header[i][1])))
		conn.send(str.encode("\r\n"))

	def _SendBody(self, conn, res):
		print("\tSend body")
		conn.send(str.encode(res.Body))

	def __del__(self):
		print("Close socket")
		self.sock.close()

if __name__ == "__main__":
	server = HTTPServer(port=8000)
	print("Server is starting!!!")
	server.Start()
	print("Server is started!!!")
	server.Listen()
