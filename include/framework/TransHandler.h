/*
 * TransHandler.h
 *
 *  Created on: 2013-5-3
 *      Author: LiuYongJin
 */

#ifndef _FRAMEWORK_TRANS_HANDLER_H_
#define _FRAMEWORK_TRANS_HANDLER_H_
#include <map>
using std::map;

#include "Logger.h"
#include "EventServer.h"
#include "IProtocolFactory.h"
#include "IAppInterface.h"

namespace easynet
{

typedef map<int32_t, ProtocolContext*> FDMap;

class TransHandler:public EventHandler
{
public:
	TransHandler(IProtocolFactory *protocol_factory)
		:m_ProtocolFactory(protocol_factory)
	{}

//基类接口方法
public:
	//时钟超时
	bool OnTimeout(){return true;}
	//io超时
	bool OnTimeout(int32_t fd);
	//可读事件
	bool OnEventRead(int32_t fd);
	//可写事件
	bool onEventWrite(int32_t fd);
	//错误事件
	bool OnEventError(int32_t fd);
private:
	//接收数据
	// @param fd          : socket fd
	// @param buffer      : 接收数据缓冲区
	// @param buffer_size : 缓冲区大小
	// @param need_size   : 预期接收的数据大小
	// @return            : 成功返回读取的字节数,错误返回-1
	int32_t ReadData(int32_t fd, char *buffer, uint32_t buffer_size, uint32_t need_size);

	//发送数据
	// @param fd          : socket fd
	// @param buffer      : 发送数据缓冲区
	// @param need_size   : 需要发送的数据大小
	// @return            : 成功返回发送的字节数,失败返回-1
	int32_t SendData(int32_t fd, char *buffer, uint32_t need_size);
private:
	FDMap m_RecvFdMap;
	FDMap m_SendFdMap;
	IAppInterface     *m_AppInterface;
	IProtocolFactory  *m_ProtocolFactory;

private:
	DECL_LOGGER(logger);
};


}//namespace
#endif //_FRAMEWORK_TRANS_HANDLER_H_

