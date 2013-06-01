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
//////////////////////////////////////////////////////////////////
//////////////////////////   接口方法   //////////////////////////
//////////////////////////////////////////////////////////////////
public:
	//启动App实例
	bool Start();

	//处理收到的请求协议
	//  @param fd             : 收到协议的socket fd
	//  @param context        : 接收到的协议上下文
	//  @param detach_context : 被设置为trues时,由应用层控制context的生存期
	//                          应用层需要在适当的时候调用DeleteProtocolContext释放context实例;
	bool OnReceiveProtocol(int32_t fd, ProtocolContext *context, bool &detach_context);

	//处理发送协议的事件.协议数据完全发送到socket的缓冲区后调本接口
	void OnSendSucc(int32_t fd, ProtocolContext *context);

	//协议数据发送到缓冲区时发生错误后调用本接口
	void OnSendError(int32_t fd, ProtocolContext *context);

	//协议数据超时未完全发送到socket后调用本接口
	void OnSendTimeout(int32_t fd, ProtocolContext *context);

	//socket发生错误调用本接口
	bool OnSocketError(int32_t fd);

	//socket读写空闲发生超时事件后调用本接口
	bool OnSocketTimeout(int32_t fd);

	//获取数据接收的超时时间(单位毫秒).从接收到协议的第一个字节开始,在该时间内如果没有收到完整的数据包将发生接收超时事件.
	int32_t GetRecvTimeoutMS();

	//获取连接空闲超时时间(单位毫秒).当连接在该时间内无任何读写事件发生的话,将发生超时事件.
	int32_t GetIdleTimeoutMS();
private:
	DECL_LOGGER(logger);
};

#endif //_ECHOSERVER_H_


