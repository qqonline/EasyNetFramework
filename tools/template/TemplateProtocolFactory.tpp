/*
 * TemplateProtocolFactory.cpp
 *
 *  Created on: CreateDate
 *      Author: AuthorName
 */

#include "TemplateProtocolFactory.h"

void TemplateProtocolFactory::InitRecvDefine(ProtocolDefine *protocol_def)
{
	//Add Your codes Here
	//

}

<<TP>>A
DecodeResult TemplateProtocolFactory::DecodeDataType(ProtocolContext *context)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}
<<TP>>END

<<TP>>AB
DecodeResult TemplateProtocolFactory::DecodeBinHeader(ProtocolContext *context)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}

DecodeResult TemplateProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}
<<TP>>END

<<TP>>AT
DecodeResult TemplateProtocolFactory::DecodeTextBody(ProtocolContext *context)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}
<<TP>>END

bool TemplateProtocolFactory::EncodeProtocol(ProtocolContext *send_context)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}

bool TemplateProtocolFactory::EncodeProtocol(void *protocol, int32_t protocol_type, char *buffer, uint32_t buffer_size)
{
	//assert(0);
	//return DECODE_SUCC;

	//Add Your codes Here
	//

	return DECODE_SUCC;
}

<<TP>>AB
void* TemplateProtocolFactory::NewProtocol(int32_t protocol_type, char *buffer, uint32_t buffer_size)
{
	//assert(0);
	//return NULL;

	//Add Your codes Here
	//

	return NULL;
}

void TemplateProtocolFactory::DeleteProtocol(void *protocol, int32_t protocol_type)
{
	//assert(0);
	//return ;

	//Add Your codes Here
	//

	return ;
}
<<TP>>END
