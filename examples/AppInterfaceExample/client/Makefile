INCLUDE=-I/usr/include/easynet -I./ -I../
LIB=-leasynet -llog4cplus

all:EchoClient

EchoClient:EchoClientMain.cpp
	g++ ${INCLUDE} ${LIB} $^ -g -o bin/$@

clean:
	rm bin/EchoClient
