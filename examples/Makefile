INCLUDE=-I/usr/include/EasyNetFramework/common
LIB=-leasynetframework
LIB_EXT=-llog4cplus
FLAGS=-g

all:Heap_example KVData_example

Heap_example:Heap_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${FLAGS} -o $@

KVData_example:KVData_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${LIB_EXT} ${FLAGS} -o $@
	
clean:
	rm Heap_example KVData_Example
