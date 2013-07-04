/*
 * Logger.h
 *
 *  Created on: Mar 14, 2013
 *      Author: LiuYongJin
 */
#ifndef _COMMON_LOGGER_H_
#define _COMMON_LOGGER_H_

#ifdef __LOG4CPLUS__
	#include <log4cplus/logger.h>
	#include <log4cplus/fileappender.h>
	#include <log4cplus/consoleappender.h>
	#include <log4cplus/layout.h>
	#include <log4cplus/loggingmacros.h>
	#include <log4cplus/configurator.h>
	using namespace log4cplus;
	#define INIT_LOGGER(conf_path)           PropertyConfigurator::doConfigure(conf_path)
	#define DECL_LOGGER(logger)              static Logger logger
	#define IMPL_LOGGER(classname, logger)   Logger classname::logger = Logger::getInstance(#classname)
	#define LOG_TRACE(logger, log)           LOG4CPLUS_TRACE(logger, log)
	#define LOG_DEBUG(logger, log)           LOG4CPLUS_DEBUG(logger, log)
	#define LOG_INFO(logger, log)            LOG4CPLUS_INFO(logger, log)
	#define LOG_WARN(logger, log)            LOG4CPLUS_WARN(logger, log)
	#define LOG_ERROR(logger, log)           LOG4CPLUS_ERROR(logger, log)
	#define LOG_FATAL(logger, log)           LOG4CPLUS_FATAL(logger, log)
#else
	#define INIT_LOGGER(conf_path)
	#define DECL_LOGGER(logger)
	#define IMPL_LOGGER(classname, logger)
	#define LOG_TRACE(logger, log)
	#define LOG_DEBUG(logger, log)
	#define LOG_INFO(logger, log)
	#define LOG_WARN(logger, log)
	#define LOG_ERROR(logger, log)
	#define LOG_FATAL(logger, log)
#endif

#endif //_COMMON_LOGGER_H_
