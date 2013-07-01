/*
 * TemplateAppInterfaceMain.cpp
 *
 *  Created on: _#CreateDate#_
 *      Author: _#AuthorName#_
 */

#include "TemplateAppInterface.h"

int main(int argc, char *argv[])
{
	PropertyConfigurator::doConfigure("../config/log4cplus.conf");

	TemplateAppInterface application;
	application.Start();
	
	return 0;
}

