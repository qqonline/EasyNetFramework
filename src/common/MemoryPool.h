/*
 * MemoryPool.h
 *
 *  Created on: 2013-5-5
 *      Author: LiuYongJin
 */
#ifndef _COMMON_MEMORY_POOL_H_
#define _COMMON_MEMORY_POOL_H_

#include <stdint.h>
#include <pthread.h>
#include <IMemory.h>

namespace easynet
{

//线程安全
class MemSlab
{
public:
	// @param elem_size : 元素大小.不小于4个字节
	// @param slab_n    : 每个块包含的元素个数
	// @param block_n   : 最大多块数.-1时表示不限制
	MemSlab(bool thread_safe, uint32_t elem_size, uint32_t slab_n=100, int32_t block_n=-1);
	~MemSlab();

	uint32_t ElementSize(){return m_ElemSize;}
	//获取一个元素内存
	void *Alloc();
	//回收元素内存
	bool Free(void *slab);
private:
	uint32_t m_ElemSize;    //元素大小
	uint32_t m_SlabNum;     //每个块包含元素个数
	void     *m_FreeList;
	void     **m_SlabArray; //块数组
	uint32_t m_Size;        //块数组大小
	int32_t  m_BlockNum;    //最大的块数
	uint32_t m_CurBlock;    //当前空闲块下标
	uint32_t m_CurIndex;    //当前空闲块中可用的元素下标

	bool m_ThreadSafe;
	pthread_mutex_t m_Mutex;
};

typedef struct _mem_info
{
	uint32_t slab_id;
	void *slab;
}MemInfo;


/**内存池:提供不同大小区间的内存块.
 *分配策略:
 *    1. 根据请求的size在对应的内存区间中的free list找一个空闲的内存块.有的话返回,否则:
 *    2. 从对应内存区间中的block中找一个内存块.有的话返回,否则重新分配一个内存block,从新的block中分配并返回.
 *    3. 超过最大内存区间的直接从系统分配内存.
 *    4. 释放时,将内存块加入到对应内存区间的free list中.
 *
 *默认配置:
 *    每个区间元素大小          : 4,8,16,32,64,128,256,512,1024,2048
 *    每个区间每个block元素个数 : 100
 *    每个区间最大的block个数   : 不限制
 *比如需要大小50字节的内存块,则将在元素大小为64的内存块中分配.
 */
class MemPool:IMemory
{
public:
	MemPool(bool thread_safe);

	// @param n             : size_array和slab_n_array,block_n_array的大小
	// @param size_array    : 指示每个MemSlab的元素大小的数组
	// @param slab_n_array  : 指示每个MemSlab中的块包含的元素个数(NULL时使用默认值)
	// @param block_n_array : 指示每个MemSlab中最多的块数(NULL或者元素值为-1时表示没有限制)
	MemPool(bool thread_safe, uint32_t n, uint32_t *size_array, uint32_t *slab_n_array=NULL, int32_t *block_n_array=NULL);

	~MemPool();

	void* Alloc(uint32_t size);
	void Free(void *mem, uint32_t size);
	void* ReAlloc(void *mem, uint32_t size, uint32_t new_size);
private:
	uint32_t m_ClassNum;
	uint32_t *m_SizeArray;
	uint32_t *m_SlabNumArray;
	int32_t  *m_BlockNumArray;

	MemSlab  **m_MemSlabArray;
	void InitMemSlab(bool thread_safe);
};

}//namespace
#endif //_COMMON_MEMORY_POOL_H_

