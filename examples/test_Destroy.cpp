/*
 * test_Destroy.cpp
 *
 *  Created on: 2013-6-30
 *      Author: tim
 */

#include "test_Destroy.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>



class B
{
public:
	B(){}
	~B(){printf("~B()\n");}
};

int main()
{
	A<int*> a;
	int i=10;
	a.Add(&i);

	A<B> a1;
	B b;
	a1.Add(b);
	return 0;
}
