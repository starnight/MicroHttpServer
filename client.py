#!/usr/bin/env python3

import unittest
import http.client

url = "localhost:8000"

class Client:
	def test_Connect(self):
		connected = 0
		try:
			self.conn = http.client.HTTPConnection(url)
			self.conn.connect()
			connected = 1
		except Exception:
			print(Exception)

		return connected
			
	def test_Close(self):
		self.conn.close()
		return 1

class TestServer(unittest.TestCase):
	def test_Scenario1(self):
		cli = Client()
		for i in range(10):
			self.assertEqual(cli.test_Connect(), 1)
			self.assertEqual(cli.test_Close(), 1)

if __name__ == "__main__":
	unittest.main()
