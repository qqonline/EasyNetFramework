INCLUDE_DIR=/usr/include/easynet/
LIBDIR=/usr/lib

FLAGS=-g
export FLAGS

OBJDIR=objects
TMPLIBDIR=lib

LIBNAME=libeasynet.a
SONAME=libeasynet.so

libs:
	(cd common/impl;$(MAKE) all)
	(cd framework/impl;$(MAKE) all)
	ar -cr ${TMPLIBDIR}/${LIBNAME} ${OBJDIR}/*.o
	g++ -fPIC -shared ${OBJDIR}/*.o -o ${TMPLIBDIR}/${SONAME}

install:
	if [ ! -d "${INCLUDE_DIR}" ];then mkdir ${INCLUDE_DIR} -p;fi
	cp common/*.h ${INCLUDE_DIR}
	cp framework/*.h ${INCLUDE_DIR}
	cp ${TMPLIBDIR}/${LIBNAME} ${LIBDIR}
	cp ${TMPLIBDIR}/${SONAME} ${LIBDIR}
	cp tools/template ${INCLUDE_DIR} -r
	cp tools/easynet.sh /usr/bin/easynet
uninstall:
	if [ -d "${INCLUDE_DIR}" ];then rm ${INCLUDE_DIR} -rf;fi
	if [ -f "${LIBDIR}/${LIBNAME}" ];then rm ${LIBDIR}/${LIBNAME} -f;fi
	if [ -f "${LIBDIR}/${SONAME}" ];then rm ${LIBDIR}/${SONAME} -f;fi
	if [ -f "/usr/bin/easynet" ];then rm /usr/bin/easynet -f;fi
clean:
	rm ${OBJDIR}/* ${TMPLIBDIR}/* 
push:
	git commit -m".." -a
	git push origin master
