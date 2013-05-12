INCLUDE=-I/usr/include/EasyNetFramework/common
LIB=-leasynet -lpthread -llog4cplus
LIB_EXT=-llog4cplus
FLAGS=-g

EXEC=Heap_example KVData_example ConfigReader_example

all:${EXEC}

Heap_example:Heap_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${FLAGS} -o $@

KVData_example:KVData_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${LIB_EXT} ${FLAGS} -o $@
	
ConfigReader_example:ConfigReader_example.cpp
	g++ $^ ${INCLUDE} ${LIB} ${LIB_EXT} ${FLAGS} -o $@

clean:
	rm ${EXEC}
