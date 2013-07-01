#FLAGS  : 由父makefile传入

INCLUDE=-I../
OBJDIR=../../objects
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all:$(OBJS)

$(OBJDIR)/%.o:%.cpp
	g++ ${INCLUDE} $< ${FLAGS} -c -o $@
