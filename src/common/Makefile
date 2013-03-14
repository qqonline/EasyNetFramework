INCLUDE=-I/usr/include/EasyNetFramework
#INCLUDE=-I/tmp/EasyNetFramework/common
FLAGS=-g

LIB=/usr/lib
LIB_NAME=libeasynetframework.a

all:lib
lib:*.cpp
	g++ ${INCLUDE} *.cpp ${FLAGS} -c
	ar -cr ${LIB_NAME} *.o 
install:
	cp ${LIB_NAME} ${LIB}
uninstall:
	rm ${LIB}/${LIB_NAME}
clean:
	rm *.o *.a
