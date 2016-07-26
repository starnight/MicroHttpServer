# Micro HTTP Server

It is a really simple HTTP server for prototyping.

One of the major purpose is that it can be ported on an embedded system (including micro controller unit level).

For writing Micro HTTP Server, the developer has studied [eserv](https://code.google.com/p/eserv/source/browse/) and [Tiny HTTPd](http://tinyhttpd.cvs.sourceforge.net/viewvc/tinyhttpd/tinyhttpd/).

## Demo

Get the code ```git clone https://github.com/starnight/MicroHttpServer.git```.

### Python Version

It will need Python 3.2 or above.

```sh
cd py-version
python3 main.py
```

Open your web browser and access the URL: http://localhost:8000

### C Version

It will need make and GCC for building.

```sh
cd c-version
make
./microhttpserver
```

Open your web browser and access the URL: http://localhost:8001

## Directories and Files

* autotest/: Place the scripts or programs for test automation.
  * client.py: The test automation program writen in Python3 tests the Micro HTTP Server.
* py-version/: Place the Python Version Micro HTTP Server.
  * main.py: The entry point of Python Version Micro HTTP Server example.
  * app.py: The web application of Python Version Micro HTTP Server example.
  * lib/: Place the Python Version Micro HTTP Server core library.
    * server.py: The Python Version Micro HTTP Server.
    * middleware.py: The Python Version Micro HTTP Server middleware.
    * \_\_init\_\_.py: Needed when server.py and middleware.py are imported by other Python programs located in other directories.
  * static/: Place the static files: HTML, JS, Images ... , which could be access directly.
* c-version/: Place the C Version Micro HTTP Server.
  * main.c: The entry point of C Version Micro HTTP Server example.
  * app.h: The web application header file of C Version Micro HTTP Server example.
  * app.c: The web application source file of C Version Micro HTTP Server example.
  * lib/: Place the C Version Micro HTTP Server core library.
    * server.h: The header file of C Version Micro HTTP Server.
    * server.c: The source file of C Version Micro HTTP Server.
    * middleware.h: The header file of C Version Micro HTTP Server middleware.
    * middleware.c: The source file of C Version Micro HTTP Server middleware.
  * static/: Place the static files: HTML, JS, Images ... , which could be access directly.
  * Makefile: The makefile of this C Version Micro HTTP Server example.
* FreeRTOS/: Place the example of Micro HTTP Server ported on FreeRTOS.
* .travis.yml: The continuous integration build script for Travis CI.
* LICENSE.md: The BSD license file.
* README.md: This read me file.

## Reference

* [RFC 2616 HTTP/1.1](https://tools.ietf.org/html/rfc2616)
* [RFC 3875 CGI](https://tools.ietf.org/html/rfc3875)
* [Wiki FastCGI](https://en.wikipedia.org/wiki/FastCGI)
* [Netscape Server Application Programming Interface (NSAPI)](https://en.wikipedia.org/wiki/Netscape_Server_Application_Programming_Interface)
* [Django & Twisted by Amber Brown @ PyCon Taiwan 2016](https://www.youtube.com/watch?v=4b3rKZTW3WA)
* [eserv](https://code.google.com/p/eserv/source/browse/)
* [Tiny HTTPd](http://tinyhttpd.cvs.sourceforge.net/viewvc/tinyhttpd/tinyhttpd/)
* [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)

## License

Micro HTTP Server's code uses the BSD license, see our **LICENSE.md** file.
