INCLUDE=-I/usr/include/easynet
LIBS=-leasynet -lpthread -llog4cplus

CFLAG=-g

all:bin/HttpEchoServer

bin/HttpEchoServer:*.cpp *.h
	g++ ${INCLUDE} $^ ${LIBS} ${CFLAG} -o $@
clean:
	rm bin/HttpEchoServer
