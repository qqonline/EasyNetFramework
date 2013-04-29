/*
 * heap_example.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: LiuYongJin
 */
#include <stdio.h>

#include "Heap.h"
using namespace easynet;

typedef struct _a_
{
	HeapItem heap_item;
	int a;
}A;

static int item_cmp(HeapItem *item0, HeapItem *item1)
{
	A *a0 = (A*)item0;
	A *a1 = (A*)item1;
	return a0->a-a1->a;
}

int main()
{
	A a[10];
	Heap heap(item_cmp);
	int i;
	for(i=9; i>=0; --i)
	{
		a[i].a = i;
		heap.Insert((HeapItem*)&a[i]);
	}

	A *temp;
	while((temp=(A*)heap.Top()) != NULL)
	{
		printf("%d ", temp->a);
		heap.Pop();
	}
	printf("\n");
	return 0;
}


