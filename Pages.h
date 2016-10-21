#ifndef PAGES_H
#define PAGES_H

#include "Error.h"
#include "Defines.h"
#include <cassert>
#include <stdlib.h>

class CHeadPage	// This class contains inverted file's information.
{
public:
	uint32_t m_pageSize;	// This file's page size.
	uint32_t dump;
	uint64_t m_pageCnt;	// This file's page count.
	uint64_t m_nextFreePage;	// The first free page's number.
	CHeadPage& operator=(const CHeadPage& var)
	{
		this->m_pageSize=var.m_pageSize;
		this->dump=var.dump;
		this->m_pageCnt=var.m_pageCnt;
		this->m_nextFreePage=var.m_nextFreePage;
		return *this;
	}
};

const unsigned int HeadPageSize=sizeof(CHeadPage);
const unsigned short PageInfoSize=sizeof(uint64_t)*2+sizeof(uint32_t);

class CPage	// This class represents every page in inverted files.
{
private:
	const uint32_t m_pageSize;	// This page's size.
	char* m_data;	// This page's data. First 8 bytes is PageNo, second 8 bytes is nextPageNo, then is real data.
public:
	CPage():m_pageSize(0),m_data(NULL){}
	void SetPageSize(const uint32_t pageSize)
	{
		*(const_cast<uint32_t*>(&this->m_pageSize))=pageSize;
		//if(this->m_data)
		//	delete [] this->m_data;
		//this->m_data=new char[pageSize];
		if(this->m_data)
		{
			free(this->m_data);
			this->m_data=NULL;
		}
		assert(0==posix_memalign((void**)&this->m_data, getpagesize(), pageSize));
		memset(this->m_data,0,pageSize);
	}
	CPage(const uint32_t pageSize);
	CPage& operator=(const CPage& var)
	{
		*(const_cast<uint32_t*>(&this->m_pageSize))=var.m_pageSize;
		this->m_data=var.m_data;
		return *this;
	}
	~CPage()
	{
		//delete [] this->m_data;
		if(this->m_data)
		{
			free(this->m_data);
			this->m_data=NULL;
		}
	}
	const uint64_t GetPageNo(void) const;
	const uint64_t GetNextPageNo(void) const;
	const uint32_t GetDateLength(void) const;
	void SetPageNo(uint64_t pageNo);
	void SetNextPageNo(uint64_t nextPageNo);
	void SetDateLength(uint32_t dataLen);
	const char* GetData() const;
	const char* GetTotalData() const
	{
		return this->m_data;
	}
	void SetData(const char* pointer,const uint32_t& length);
	void ZeroData(void)
	{
		memset(this->m_data,0,this->m_pageSize);
	}
};

class CIndexData
{
private:
	uint32_t m_length;
	char* m_data;
public:
	CIndexData():m_length(0),m_data(NULL){}
	CIndexData(const uint32_t len)
	{
		this->m_length=len;
		this->m_data=new char[len];
	}
	CIndexData(const uint32_t len,const char* pointer);
	CIndexData& operator=(const CIndexData& var)
	{
		assert(this->m_length==var.m_length);
		memcpy(this->m_data,var.m_data,this->m_length);
		return *this;
	}
	const uint32_t GetLength() const
	{
		return this->m_length;
	}
	const char* GetData() const
	{
		return this->m_data;
	}
	void SetLength(uint32_t len)
	{
		if(this->m_data)
			delete [] this->m_data;
		this->m_length=len;
		this->m_data=new char[len];
	}
	~CIndexData();
};

class CPagesTest
{
public:
	void RunTest(void) const;
};

#endif
