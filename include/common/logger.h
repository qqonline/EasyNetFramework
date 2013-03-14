/*
 * logger.h
 *
 *  Created on: Mar 14, 2013
 *      Author: LiuYongJin
 */
#ifndef _COMMON_LOGGER_H_
#define _COMMON_LOGGER_H_

#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>
using namespace log4cplus;

#include <list>
using namespace std;

#define DECL_LOGGER(logger)     static Logger logger
#define IMPL_LOGGER(classname, logger)  Logger classname::logger = Logger::getInstance(#classname)

#endif //_COMMON_LOGGER_H_
