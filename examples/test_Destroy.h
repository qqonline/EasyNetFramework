/*
 * test_Destroy.h
 *
 *  Created on: 2013-6-30
 *      Author: tim
 */

#ifndef _TEST_DESTROY_H_
#define _TEST_DESTROY_H_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <bits/stl_iterator_base_types.h>
#include <bits/stl_construct.h>
using namespace std;

template <typename T>
class A
{
public:
	A():m_Index(0)
	{
		m_Array = (T*)malloc(sizeof(T)*10);
	}
	~A()
	{
		printf("~A()\n");
		if(m_Index > 0)
		{
			std::_Destroy(&m_Array[0], &m_Array[m_Index]);
		}
		free(m_Array);
	}

	void Add(const T &value)
	{
		assert(m_Index < 10);
		m_Array[m_Index++] = value;
	}
private:
	int m_Index;
	T* m_Array;
};

#endif
