#!/usr/bin/env python3

import unittest
import http.client
import sys
import random

server = "localhost:8000"

def fib(level):
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

class Client:
	def test_Connect(self, server):
		connected = 0
		try:
			self.conn = http.client.HTTPConnection(server)
			self.conn.connect()
			connected = 1
		except Exception:
			print(Exception)

		return connected

	def test_GetRequest(self, uri="/"):
		res = None
		self.conn.request("GET", uri)
		res = self.conn.getresponse()
		return res
	
	def test_PostRequest(self, uri="/", body=None):
		res = None
		self.conn.request("POST", uri, body)
		res = self.conn.getresponse()
		return res

	def test_Close(self):
		self.conn.close()
		return 1

class TestServer(unittest.TestCase):
	def setUp(self):
		if len(sys.argv) >= 2:
			server = sys.argv[1]
			print(server)

	def test_Scenario1(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario2(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequest()
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			self.assertEqual(res.read(22), b"<html><body>Hello!<br>")
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario3(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequest("/index.htm")
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 404)
			self.assertEqual(res.read(22), b"")
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario4(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequest("/index.html")
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			self.assertEqual(res.read(22), b"<html><body>Hello!<br>")
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario5(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequest("/sample.html")
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			pattern = "<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n</head>\n<body>\nThis is sample page.<br>\n許功蓋\n</body>\n</html>"
			self.assertEqual(res.read(len(str.encode(pattern))), str.encode(pattern))
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario6(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_PostRequest("/index.html", None)
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 404)
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario7(self):
		cli = Client()
		for i in range(40):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_PostRequest("/fib", str.encode("Level={}".format(str(i))))
			pattern = str(fib(i))
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			self.assertEqual(res.read(len(str.encode(pattern))), str.encode(pattern))
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario8(self):
		cli = Client()
		for i in range(40):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_PostRequest("/fib", str.encode("Level={}".format(str(i)+"a")))
			pattern = str(fib(i))
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			self.assertEqual(res.read(len(str.encode(pattern))), str.encode(pattern))
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario9(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequest("/fib")
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 404)
			self.assertEqual(cli.test_Close(), 1)

if __name__ == "__main__":
	if len(sys.argv) >= 2:
		server = sys.argv.pop()

	unittest.main()
