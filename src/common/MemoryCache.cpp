/*
 * MemoryCache.cpp
 *
 *  Created on: 2013-5-5
 *      Author: LiuYongJin
 */
#include <stdlib.h>
#include <assert.h>

#include "MemoryCache.h"

namespace easynet
{

MemSlab::MemSlab(uint32_t elem_size, uint32_t slab_n)
{
	if(elem_size < 4)    //至少4个字节
		elem_size = 4;
	m_ElemSize  = elem_size;
	m_SlabNum   = slab_n;
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

void* MemSlab::Get()
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

bool MemSlab::Recycle(void *slab)
{
	if(slab == NULL)
		return true;
	*(void**)slab = m_FreeList;
	*(void**)m_FreeList = slab;
	return true;
}


/////////////////////// MemCache /////////////////////
static uint32_t _defaut_size[10] = {4,8,16,32,64,128,256,512,1024,2048};
static uint32_t _defaut_num[10]  = {100,100,100,100,100,100,100,100,100,100};

MemCache::MemCache()
{
	m_ClassNum = 10;
	m_SizeArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_SlabNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	memcpy(m_SizeArray, _defaut_size, m_ClassNum*sizeof(uint32_t));
	memcpy(m_SlabNumArray, _defaut_num, m_ClassNum*sizeof(uint32_t));

	InitMemSlab();
}

MemCache::MemCache(uint32_t n, uint32_t *size_array, uint32_t *slab_n_array=NULL)
{
	m_ClassNum = n;
	m_SizeArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	m_SlabNumArray = (uint32_t*)malloc(m_ClassNum*sizeof(uint32_t));
	memcpy(m_SizeArray, size_array, m_ClassNum*sizeof(uint32_t));
	memcpy(m_SlabNumArray, slab_n_array, m_ClassNum*sizeof(uint32_t));

	InitMemSlab();
}

void MemCache::InitMemSlab()
{
	m_MemSlabArray = (MemSlab**)malloc(m_ClassNum*sizeof(MemSlab));
	int i;
	for(i=0; i<m_ClassNum; ++i)
		m_MemSlabArray[i] = new MemSlab(m_SizeArray[i], m_SlabNumArray[i]);
}

MemCache::~MemCache()
{
	int i;
	for(i=0; i<m_ClassNum; ++i)
		delete m_MemSlabArray[i];
	free(m_MemSlabArray);
	free(m_SizeArray);
	free(m_SlabNumArray);
}


bool MemCache::Get(uint32_t size, MemInfo *mem_info)
{
	if(mem_info == NULL)
		return false;

	int slab_id;
	void *slab = NULL;

	for(slab_id=0; slab_id<m_ClassNum; ++slab_id)
	{
		if(size <= m_SizeArray[slab_id])
			break;
	}

	if(slab_id < m_ClassNum)
		slab = m_MemSlabArray[slab_id]->Get();  //从mem slab中分配
	else
		slab = malloc(size);              //内存太大, 直接分配

	if(slab == NULL)
		return false;
	mem_info->slab_id = slab_id;
	mem_info->slab    = slab;
	return true;
}

//回收内存
bool MemCache::Recycle(MemInfo *mem_info)
{
	if(mem_info==NULL || mem_info->slab==NULL)
		return true;
	if(mem_info->slab_id < m_ClassNum)
		m_MemSlabArray[mem_info->slab_id]->Recycle(mem_info->slab);
	else
		free(mem_info->slab);
	return true;
}

}//namespace