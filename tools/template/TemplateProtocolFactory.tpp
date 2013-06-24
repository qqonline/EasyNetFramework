/*
 * TemplateProtocolFactory.cpp
 *
 *  Created on: _#CreateDate#_
 *      Author: _#AuthorName#_
 */

#include "TemplateProtocolFactory.h"

uint32_t TemplateProtocolFactory::HeaderSize()
{
	////Add Your Code Here

}

DecodeResult TemplateProtocolFactory::DecodeHeader(const char *buffer, DataType &type, uint32_t &body_size)
{
	////Add Your Code Here
	
	return DECODE_SUCC;
}

void TemplateProtocolFactory::EncodeHeader(char *buffer, uint32_t body_size)
{
	////Add Your Code Here

	return ;
}

DecodeResult TemplateProtocolFactory::DecodeBinBody(ProtocolContext *context)
{
	////Add Your Code Here

	return DECODE_SUCC;
}

DecodeResult TemplateProtocolFactory::DecodeTextBody(ProtocolContext *context)
{
	////Add Your Code Here

	return DECODE_SUCC;
}

void TemplateProtocolFactory::DeleteProtocol(uint32_t protocol_type, void *protocol)
{
	////Add Your Code Here

	return ;
}

