# Micro HTTP Server

It is a really simple HTTP server for prototyping.

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

It will need make and GCC.

```sh
cd c-version
make
./microhttpserver
```

Open your web browser and access the URL: http://localhost:8001

## Reference

* [RFC 2616](https://tools.ietf.org/html/rfc2616)
* [eserv](https://code.google.com/p/eserv/source/browse/)
* [Tiny HTTPd](http://tinyhttpd.cvs.sourceforge.net/viewvc/tinyhttpd/tinyhttpd/)
