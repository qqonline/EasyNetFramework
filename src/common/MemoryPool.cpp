/*
 * MemoryCache.cpp
 *
 *  Created on: 2013-5-5
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <assert.h>

#include "MemoryPool.h"

namespace easynet
{

MemSlab::MemSlab(uint32_t elem_size, uint32_t slab_n, int32_t block_n)
{
	if(elem_size < 4)    //至少4个字节
		elem_size = 4;
	m_ElemSize  = elem_size;
	m_SlabNum   = slab_n;
	m_BlockNum = block_n;
	m_FreeList  = NULL;

	m_Size      = 100;
	m_SlabArray = (void**)calloc(m_Size, sizeof(void*));

	m_CurBlock  = 0;
	m_CurIndex  = 0;
	void *block = calloc(m_SlabNum, m_ElemSize);
	assert(block != NULL);
	m_SlabArray[m_CurBlock] = block;
}

MemSlab::~MemSlab()
{
	int i;
	for(i=0; i<m_CurBlock; ++i)
		free(m_SlabArray[i]);
	free(m_SlabArray);
}

void* MemSlab::Alloc()
{
	void *temp = NULL;
	if(m_FreeList != NULL)    //从空闲队列中获取
	{
		temp = m_FreeList;
		m_FreeList = *(void**)m_FreeList;
	}
	else    //从空闲块中获取
	{
		if(m_CurIndex == m_SlabNum)  //块已经使用完
		{
			if(m_BlockNum>-1 && m_CurBlock==m_BlockNum-1)    //达到最大块数
				return NULL;

			//分配新块
			++m_CurBlock;
			if(m_CurBlock == m_Size) //数组空间已经使用完
			{
				void **temp = (void**)realloc(m_SlabArray, m_Size*2*sizeof(void*));
				if(temp == NULL)
					return NULL;
				m_SlabArray = temp;
				m_Size *= 2;
			}
			void *block = calloc(m_SlabNum, m_ElemSize);    //重新分配一个块
			if(block == NULL)
				return NULL;
			m_SlabArray[m_CurBlock] = block;
			m_CurIndex = 0;
		}
		char *block = (char*)m_SlabArray[m_CurBlock];
		temp = (void*)(block+m_CurIndex*m_ElemSize);
		++m_CurIndex;
	}
	return temp;
}

bool MemSlab::Free(void *slab)
{
	if(slab == NULL)
		return true;
	*(void**)slab = m_FreeList;
	*(void**)m_FreeList = slab;
	return true;
}


/////////////////////// MemCache /////////////////////
#define DEFUALT_SIZE 10
static uint32_t _defaut_size[DEFUALT_SIZE] = {4,8,16,32,64,128,256,512,1024,2048};
static uint32_t _defaut_num[DEFUALT_SIZE]  = {100,100,100,100,100,100,100,100,100,100};
static uint32_t _defaut_block_num[DEFUALT_SIZE]  = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

MemPool::MemPool()
{
	m_ClassNum = DEFUALT_SIZE;
	m_SizeArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_SlabNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_BlockNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));

	memcpy(m_SizeArray, _defaut_size, m_ClassNum*sizeof(uint32_t));
	memcpy(m_SlabNumArray, _defaut_num, m_ClassNum*sizeof(uint32_t));
	memcpy(m_BlockNumArray, _defaut_block_num, m_ClassNum*sizeof(uint32_t));

	InitMemSlab();
}

MemPool::MemPool(uint32_t n, uint32_t *size_array, uint32_t *slab_n_array/*=NULL*/, int32_t *block_n_array/*=NULL*/)
{
	m_ClassNum = n;
	m_SizeArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_SlabNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_BlockNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));

	memcpy(m_SizeArray, size_array, m_ClassNum*sizeof(uint32_t));
	if(slab_n_array != NULL)
		memcpy(m_SlabNumArray, slab_n_array, m_ClassNum*sizeof(uint32_t));
	else
		memcpy(m_SlabNumArray, _defaut_num, m_ClassNum*sizeof(uint32_t));

	if(block_n_array != NULL)
		memcpy(m_BlockNumArray, block_n_array, m_ClassNum*sizeof(uint32_t));
	else
		memcpy(m_BlockNumArray, _defaut_block_num, m_ClassNum*sizeof(uint32_t));

	InitMemSlab();
}

void MemPool::InitMemSlab()
{
	m_MemSlabArray = (MemSlab**)malloc(m_ClassNum*sizeof(MemSlab));
	int i;
	for(i=0; i<m_ClassNum; ++i)
		m_MemSlabArray[i] = new MemSlab(m_SizeArray[i], m_SlabNumArray[i]);
}

MemPool::~MemPool()
{
	int i;
	for(i=0; i<m_ClassNum; ++i)
		delete m_MemSlabArray[i];
	free(m_MemSlabArray);
	free(m_SizeArray);
	free(m_SlabNumArray);
}


void* MemPool::Alloc(uint32_t size)
{
	int slab_id;
	void *slab = NULL;

	for(slab_id=0; slab_id<m_ClassNum; ++slab_id)
	{
		if(size <= m_SizeArray[slab_id])
			break;
	}

	if(slab_id < m_ClassNum)
		slab = m_MemSlabArray[slab_id]->Alloc();  //从mem slab中分配
	else
		slab = malloc(size);              //内存太大, 直接分配

	return slab;
}

//回收内存
bool MemPool::Free(void *data, uint32_t size)
{
	if(data==NULL)
		return true;
	uint32_t slab_id;
	for(slab_id=0; slab_id<m_ClassNum; ++slab_id)
	{
		if(size <= m_SizeArray[slab_id])
			break;
	}
	if(slab_id < m_ClassNum)
		return m_MemSlabArray[slab_id]->Free(data);
	else
		free(data);
	return true;
}

}//namespace
