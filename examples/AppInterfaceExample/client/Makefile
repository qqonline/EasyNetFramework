INCLUDE=-I/usr/include/easynet -I./ -I../
LIB=-leasynet -llog4cplus

all:bin/EchoClient

bin/EchoClient:EchoClientMain.cpp
	g++ ${INCLUDE} ${LIB} $^ -g -o $@

clean:
	rm bin/EchoClient
