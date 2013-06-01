INCLUDE=-I/usr/include/easynet -I./ -I../
#LIB=-leasynet -llog4cplus
LIB=/usr/lib/libeasynet.a -llog4cplus

all:EchoServer
rebuild:clean EchoServer

EchoServer:EchoServer.cpp EchoServer.h EchoServerMain.cpp
	g++ ${INCLUDE} $^ ${LIB} -g -o $@

clean:
	rm EchoServer
