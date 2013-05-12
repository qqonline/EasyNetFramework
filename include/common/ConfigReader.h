/*
 * ConfigReader.h
 *
 *  Created on: 2012-12-13
 *      Author: LiuYongJin
 */


#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
using std::map;
using std::string;

namespace easynet
{


//参数行最大长度
#define LINE_LEN 256

//配置文件读取工具
/*注释字符: '#'和'/'
            出现在一行的其他位置,该位置之后的内容将被忽略掉,出现在行首,整行被忽略掉;
 *key-value分割符: '='
            key和value与分割符中间可以出现任意的空白符
 *允许出现相同的key:
            当初现相同的key时,后一个key的value将替换前一个的value
 *value的类型:整数类型和字符串类型
 *配置文件内容修改后,允许重新加载
*/

typedef map<string, string> KeyValueMap;
class ConfigReader
{
public:
	ConfigReader(const char *config_path)
	{
		Init(config_path);
	}

	bool Init(const char *config_path);
	string ShowKeyValue();

	string GetValue(const char *key, const string &default_value="");
	string GetValue(const char *key, const char *default_value="");
	int GetValue(const char *key, int default_value=0);
private:
	KeyValueMap m_KayValueMap;
};

inline
bool ConfigReader::Init(const char *config_path)
{
	m_KayValueMap.clear();

	FILE *fp = fopen(config_path, "r");
	if(fp == NULL)
		return false;

	char buf[LINE_LEN];
	while(fgets(buf,LINE_LEN,fp) != NULL)    //read one line
	{
		char *temp = buf;
		//1. parse key
		while(*temp!='\0' && (*temp==' ' || *temp=='\t'))    //跳过行首空白
			++temp;
		if(*temp=='\0' || *temp=='\r' || *temp=='\n' || *temp=='#' || *temp=='/')    //空白行或者注释行
			continue;

		char *key = temp++;
		while(*temp!='\0' && *temp!=' ' && *temp!='\t' && *temp!='=')    //跳过key直到遇到空白符或者‘=’
			++temp;
		if(*temp == '\0')    //找不到空白符或者"="号
			continue;

		//2. parse "="
		while(*temp==' ' || *temp=='\t')    //将空格设置成’\0‘
			*temp++='\0';
		if(*temp != '=')    //没有找到"="号
			continue;
		*temp++='\0';

		//3. parse value
		while(*temp!='\0' && (*temp==' ' || *temp=='\t'))    //跳过‘=’和value之间的空白符
			++temp;
		if(*temp=='\0' || *temp=='\r' || *temp=='\n' || *temp=='#' || *temp=='/')    //找不到value
			continue;
		char *value = temp++;
		while(*temp!='\0' && *temp!=' ' && *temp!='\t' && *temp!='#' && *temp!='/' && *temp!='\r' && *temp!='\n')    //找到value的结束位置
			++temp;
		*temp = '\0';

		//4. save key-value
		KeyValueMap::iterator it = m_KayValueMap.find(key);
		if(it == m_KayValueMap.end())    //新的key-value
			m_KayValueMap.insert(std::make_pair(key, value));
		else
			it->second = value;    //用新的替换旧的
	}
	fclose(fp);
	return true;
}

inline
string ConfigReader::GetValue(const char *key, const string &default_value)
{
	KeyValueMap::iterator it = m_KayValueMap.find(key);
	if(it != m_KayValueMap.end())
		return it->second;
	return default_value;
}

inline
string ConfigReader::GetValue(const char *key, const char *default_value)
{
	KeyValueMap::iterator it = m_KayValueMap.find(key);
	if(it != m_KayValueMap.end())
		return it->second;
	if(default_value == NULL)
		return "";
	else
		return default_value;
}

inline
int ConfigReader::GetValue(const char *key, int default_value)
{
	KeyValueMap::iterator it = m_KayValueMap.find(key);
	if(it != m_KayValueMap.end())
	{
		int temp;
		if(sscanf(it->second.c_str(), "%d", &temp) > 0)
			return temp;
	}
	return default_value;
}

inline
string ConfigReader::ShowKeyValue()
{
	string kv="";
	KeyValueMap::iterator it;
	for(it=m_KayValueMap.begin(); it!=m_KayValueMap.end(); ++it)
	{
		kv += "["+it->first+"="+it->second+"] ";
	}
	return kv;
}


}//namespace
#endif //_CONFIG_READER_H_
