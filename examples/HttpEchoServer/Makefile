INCLUDE=-I/usr/include/easynet
LIBS=-leasynet -lpthread -llog4cplus

CFLAG=-g -D__LOG4CPLUS__

all:bin/HttpEchoServer

bin/HttpEchoServer:*.cpp *.h
	g++ ${INCLUDE} $^ ${LIBS} ${CFLAG} -o $@
clean:
	rm bin/HttpEchoServer
