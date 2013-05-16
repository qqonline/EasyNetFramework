INCLUDE_DIR=/usr/include/easynet/
LIBDIR=/usr/lib

FLAGS=-g
export FLAGS

OBJDIR=src/objects
TMPLIBDIR=src/lib

LIBNAME=libeasynet.a
SONAME=libeasynet.so

lib:
	(cd src/common;$(MAKE) all)
	(cd src/framework;$(MAKE) all)
	ar -cr ${TMPLIBDIR}/${LIBNAME} ${OBJDIR}/*.o
	g++ -fPIC -shared ${OBJDIR}/*.o -o ${TMPLIBDIR}/${SONAME}

install:
	if [ ! -d "${INCLUDE_DIR}" ];then mkdir ${INCLUDE_DIR} -p;fi
	cp include/common/* ${INCLUDE_DIR}
	cp include/framework/* ${INCLUDE_DIR}
	cp ${TMPLIBDIR}/${LIBNAME} ${LIBDIR}
	cp ${TMPLIBDIR}/${SONAME} ${LIBDIR}
 
uninstall:
	if [ -d "${INCLUDE_DIR}" ];then rm ${INCLUDE_DIR} -rf;fi
	if [ -f "${LIBDIR}/${LIBNAME}" ];then rm ${LIBDIR}/${LIBNAME} -f;fi
	if [ -f "${LIBDIR}/${SONAME}" ];then rm ${LIBDIR}/${SONAME} -f;fi
clean:
	rm ${OBJDIR}/* ${TMPLIBDIR}/* 
push:
	git commit -m".." -a
	git push origin master
