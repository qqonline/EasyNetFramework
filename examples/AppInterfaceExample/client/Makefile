INCLUDE=-I/usr/include/easynet -I./ -I../
LIB=-leasynet -llog4cplus
FLAGS=-g -D__LOG4CPLUS__
all:bin/EchoClient

bin/EchoClient:EchoClientMain.cpp
	g++ ${INCLUDE} ${LIB} $^ ${FLAGS} -o $@

clean:
	rm bin/EchoClient
