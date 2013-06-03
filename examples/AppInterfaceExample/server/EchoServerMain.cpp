/*
 * EchoServerMain.cpp
 *
 *  Created on: 2013-05-31
 *      Author: tim
 */

#include "EchoServer.h"

int main(int argc, char *argv[])
{
	PropertyConfigurator::doConfigure("../config/log4cplus.conf");

	EchoServer application;
	application.Start();
	
	return 0;
}

