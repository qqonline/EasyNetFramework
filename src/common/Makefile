#INCLUDE=-I/usr/include/EasyNetFramework
INCLUDE=-I../../include/common
FLAGS=-g

LIBDIR=/usr/lib
OBJDIR=../objects
TMPLIBDIR=../lib
LIBNAME=libeasynet.a
SONAME=libeasynet.so

SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all:lib share
lib:$(OBJS)
	ar -cr ${TMPLIBDIR}/${LIBNAME} $^
share:$(OBJS)
	g++ -fPIC -shared $^ -o ${TMPLIBDIR}/${SONAME}
$(OBJDIR)/%.o:%.cpp
	g++ ${INCLUDE} $< ${FLAGS} -c -o $@
install:
	cp ${TMPLIBDIR}/${LIBNAME} ${LIBDIR}
	cp ${TMPLIBDIR}/${SONAME}  ${LIBDIR}
uninstall:
	rm ${LIBDIR}/${LIBNAME} ${LIBDIR}/${SONAME}
clean:
	rm $(OBJDIR)/* ${TMPLIBDIR}/*
