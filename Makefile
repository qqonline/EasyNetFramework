INCLUDE_DIR=/usr/include/EasyNetFramework
#INCLUDE_DIR=/tmp/EasyNetFramework

lib:
	(cd src/common;$(MAKE) all)

install:
	@if [ ! -d "${INCLUDE_DIR}/common" ];then mkdir ${INCLUDE_DIR}/common -p;fi
	cp include/common/* ${INCLUDE_DIR}/common/
	(cd src/common;$(MAKE) install)

uninstall:
	@if [ -d "${INCLUDE_DIR}" ];then	rm ${INCLUDE_DIR} -rf;fi
	(cd src/common; $(MAKE) uninstall)

push:
	git commit -m".." -a
	git push origin master
