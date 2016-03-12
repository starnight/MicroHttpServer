#!/usr/bin/env python3

import unittest
import http.client
import sys

server = "localhost:8000"

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

	def test_GetRequst(self, uri="/"):
		res = None
		self.conn.request("GET", uri)
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
		for i in range(10):
			cli = Client()
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequst()
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 200)
			self.assertEqual(res.read(22), b"<html><body>Hello!<br>")
			self.assertEqual(cli.test_Close(), 1)

	def test_Scenario3(self):
		for i in range(10):
			cli = Client()
			self.assertEqual(cli.test_Connect(server), 1)
			res = cli.test_GetRequst("/index.htm")
			self.assertIsNotNone(res)
			self.assertEqual(res.status, 404)
			self.assertEqual(res.read(22), b"")
			self.assertEqual(cli.test_Close(), 1)

if __name__ == "__main__":
	if len(sys.argv) >= 2:
		server = sys.argv.pop()

	unittest.main()
