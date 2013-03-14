INCLUDE=-I/usr/include/EasyNetFramework
LIB=-leasynetframework
FLAGS=-g

all:heap_example

heap_example:heap_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${FLAGS} -o $@

clean:
	rm heap_example
