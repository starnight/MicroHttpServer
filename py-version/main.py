#!/usr/bin/env python3

from lib.server import HTTPServer
from lib.middleware import Routes
import app

if __name__ == "__main__":
	print("Server is starting!!!")
	server = HTTPServer(port=8000)
	routes = Routes()
	routes.AddRoute("GET", "/", app.WellcomePage)
	routes.AddRoute("GET", "/index.html", app.WellcomePage)
	routes.AddRoute("POST", "/fib", app.Fib)
	print("Server is started!!!")
	server.RunLoop(routes.Dispatch)
