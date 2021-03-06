/*
 * EchoServer.h
 *
 *  Created on: 2013-05-31
 *      Author: tim
 */

#ifndef _ECHOSERVER_H_
#define _ECHOSERVER_H_

#include "IAppInterface.h"
using namespace easynet;

//默认使用以下组件实例:
//    EventServer     : EventServerEpoll
//    ProtocolFactory : KVDataProtocolFactory
//    TransHandler    : TransHandler
//    ListenHandler   : ListenHandler
//    IMemory         : SystemMemory
class EchoServer:public IAppInterface
{
public:
	void OnTimeout(uint64_t nowtime_ms);    //响应Timer的超时事件

//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//启动App实例
	bool Start();

	//获取数据接收的超时时间(单位毫秒).从数据开始接收到收到完整的数据包所允许的时间.
	int32_t GetSocketRecvTimeout();

	//获取连接空闲的超时时间(单位毫秒秒).当连接在该时间内无任何读写事件发生的话,将发生超时事件.
	int32_t GetSocketIdleTimeout();

	//获取允许的最大链接数.
	int32_t GetMaxConnections();


	//处理收到的请求协议
	//  @param fd             : 收到协议的socket
	//  @param context        : 接收到的协议上下文
	//  @param detach_context : 被设置为trues时,由应用层控制context的生存期
	//                          应用层需要在适当的时候调用DeleteProtocolContext释放context实例;
	bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context);

	//处理发送协议的事件.协议数据完全发送到socket的缓冲区后调本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送成功的数据,应用层需要根据创建方式对齐进行释放
	void OnSendSucc(int32_t fd, ProtocolContext *context);

	//协议数据发送到缓冲区时发生错误后调用本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送失败的数据,应用层需要根据创建方式对齐进行释放
	void OnSendError(int32_t fd, ProtocolContext *context);

	//协议数据超时未完全发送到socket后调用本接口
	//  @param fd             : 发送数据的socket
	//  @param context        : 发送超时的数据,应用层需要根据创建方式对齐进行释放
	void OnSendTimeout(int32_t fd, ProtocolContext *context);

	//socket需要结束时调用本接口
	//  @param fd             : 需要结束的socket
	void OnSocketFinished(int32_t fd);
private:
	DECL_LOGGER(logger);
};

#endif //_ECHOSERVER_H_


