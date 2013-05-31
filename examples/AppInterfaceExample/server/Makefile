INCLUDE=-I/usr/include/easynet -I./ -I../
LIB=-leasynet -llog4cplus

all:EchoServer

EchoServer:EchoServer.cpp EchoServer.h EchoServerMain.cpp
	g++ ${INCLUDE} ${LIB} $^ -g -o $@

clean:
	rm EchoServer
