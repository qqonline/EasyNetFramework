/*
 * MemoryPool.h
 *
 *  Created on: 2013-5-5
 *      Author: LiuYongJin
 */
#ifndef _COMMON_MEMORY_POOL_H_
#define _COMMON_MEMORY_POOL_H_

#include <stdint.h>

namespace easynet
{

class MemSlab
{
public:
	// @param elem_size : 元素大小.不小于4个字节
	// @param slab_n    : 每个块包含的元素个数
	MemSlab(uint32_t elem_size, uint32_t slab_n=100);
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
	uint32_t m_CurBlock;    //当前空闲块下标
	uint32_t m_CurIndex;    //当前空闲块中可用的元素下标
};

typedef struct _mem_info
{
	uint32_t slab_id;
	void *slab;
}MemInfo;

class MemPool
{
public:
	//默认的MemSlab元素大小为:4,8,16,32,64,128,256,512,1024,2048
	MemPool();
	// @param n            : size_array和slab_n_array的大小
	// @param size_array   : 指示每个MemSlab的元素大小的数组
	// @param slab_n_array : 指示每个MemSlab中的块包含的元素个数(为NULL时使用默认值)
	MemPool(uint32_t n, uint32_t *size_array, uint32_t *slab_n_array=NULL);

	~MemCache();

	//获取大小为size的内存,成功返回true,meminfo有效;失败返回flase;
	bool Alloc(uint32_t size, MemInfo *mem_info);
	//回收内存
	bool Free(MemInfo *mem_info);
private:
	uint32_t m_ClassNum;
	uint32_t *m_SizeArray;
	uint32_t *m_SlabNumArray;

	MemSlab  **m_MemSlabArray;
	void InitMemSlab();
};

}//namespace
#endif //_COMMON_MEMORY_POOL_H_

