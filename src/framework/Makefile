#FLAGS  : 由父makefile传入

INCLUDE=-I../../include/common -I../../include/framework
OBJDIR=../objects
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all:$(OBJS)

$(OBJDIR)/%.o:%.cpp
	g++ ${INCLUDE} $< ${FLAGS} -c -o $@

