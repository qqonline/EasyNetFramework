#include "ConfigReader.h"

#include <string>
using std::string;

int main()
{
	easynet::ConfigReader config_reader("./conf/ConfigReader.conf");

	printf("test1 1:\n");
	string kv = config_reader.ShowKeyValue();
	printf("show config key-value: %s \n", kv.c_str());

	printf("\ntest 2:\n");

	int a = config_reader.GetValue("a", -1);
	printf("a = %d\n", a);

	string b = config_reader.GetValue("b", "default value");
	printf("b = \"%s\"\n", b.c_str());
	
	string c = config_reader.GetValue("c", "default value");
	printf("c = \"%s\"\n", c.c_str());
	return 0;
}
