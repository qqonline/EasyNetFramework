INCLUDE=-I/usr/include/easynet -I./ -I../
#LIB=-leasynet -llog4cplus
LIB=/usr/lib/libeasynet.a -llog4cplus
FLAGS=-g -D__LOG4CPLUS__
all:bin/EchoServer
rebuild:clean bin/EchoServer

bin/EchoServer:EchoServer.cpp EchoServer.h EchoServerMain.cpp
	g++ ${INCLUDE} $^ ${LIB} ${FLAGS} -o $@

clean:
	rm bin/EchoServer
