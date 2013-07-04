/*
 * HttpEchoServerMain.cpp
 *
 *  Created on: 2013-06-24
 *      Author: tim
 */

#include "HttpEchoServer.h"

int main(int argc, char *argv[])
{
	INIT_LOGGER("../config/log4cplus.conf");

	HttpEchoServer application;
	application.Start();
	
	return 0;
}

