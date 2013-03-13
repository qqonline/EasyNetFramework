#INCLUDE_DIR=/usr/include/EasyNetFramework
INCLUDE_DIR=/tmp/EasyNetFramework

install:
	mkdir ${INCLUDE_DIR}/common -p
	cp include/common/* ${INCLUDE_DIR}/common/

uninstall:
	rm ${INCLUDE_DIR} -rf
