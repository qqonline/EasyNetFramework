INCLUDE=-I/usr/include/EasyNetFramework
LIB=-leasynetframework
LIB_EXT=-llog4cplus
FLAGS=-g

all:heap_example iodemuxer_example

heap_example:heap_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${FLAGS} -o $@

iodemuxer_example:iodemuxer_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${LIB_EXT} ${FLAGS} -o $@
	
clean:
	rm heap_example iodemuxer_example
