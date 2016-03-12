#!/usr/bin/env python3

import mimetypes

class Routes:
	'''Define the feature of route for URIs.'''
	def __init__(self):
		self._Routes = []
		mimetypes.init()

	def AddRoute(self, uri, callback):
		'''Add an URI into the route table.'''
		self._Routes.append([uri, callback])

	def Dispatch(self, req, res):
		'''Dispatch an URI according to the route table.'''
		uri = ""
		for fv in req.Header:
			if fv[0] == "URI":
				uri = fv[1]
				found = 1
				break

		found = 0
		# Check the route
		for r in self._Routes:
			if r[0] == uri:
				r[1](req, res)
				found = 1
				break
		# Check static files
		if found != 1:
			found = self._ReadStaticFiles(uri, res)
		# It is really not found
		if found != 1:
			self._NotFound(req, res)

	def _ReadStaticFiles(self, uri, res):
		found = 0

		# Prevent Path Traversal
		depth = 0
		for d in uri.split("/"):
			if d == "..":
				depth -= 1
			elif d =="":
				continue
			else:
				depth += 1
			if depth < 0:
				break

		if depth >= 0:
			# Try to open and load the static file.
			try:
				filename = "static/{}".format(uri)
				f = open(filename, "r")
				res.Body = f.read()
				f.close()
				mime = mimetypes.guess_type(filename)[0]
				res.Header.append(["Content-Type", mime])
				found = 1
			except:
				pass

		return found

	def _NotFound(self, req, res):
		'''Define the default error page for not found URI.'''
		res.Header.append(["Status", "404 Not Found"])
