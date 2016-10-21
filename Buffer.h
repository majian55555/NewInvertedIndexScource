#ifndef BUFFER_H
#define BUFFER_H

#include "FileOp.h"
#include <vector>
const int64_t MinBufferNum=50*1024*1024;

class CBufferIter
{
	friend class CBufferMgr;
private:
	bool m_dirty;
	bool m_refbit;
	bool m_valid;
	unsigned short m_pinCnt;
	uint64_t m_bufferID;
public:
	CBufferIter():m_dirty(false),m_refbit(false),m_valid(true),m_pinCnt(0),m_bufferID(0){}
	void SetDirty(void)
	{
		this->m_dirty=true;
	}
	const bool GetDirty(void)
	{
		return this->m_dirty;
	}
	void SetRefbit(const bool refbit)
	{
		this->m_refbit=refbit;
	}
	const bool GetRefbit(void)
	{
		return this->m_refbit;
	}
	const uint64_t GetBufferID() const
	{
		return this->m_bufferID;
	}
	void SetBufferIter(const uint64_t& bufferID)
	{
		this->m_bufferID=bufferID;
	}
	const unsigned short GetPinCnt() const
	{
		return this->m_pinCnt;
	}
	void AddPinCnt(void)
	{
		this->m_pinCnt++;
	}
	void UnPin(void)
	{
		if(this->m_pinCnt>0)
			this->m_pinCnt--;
	}
	const bool GetValid(void) const
	{
		return this->m_valid;
	}
	void SetValid(const bool valid)
	{
		this->m_valid=valid;
	}
};

const uint32_t CBufferIterSize=sizeof(CBufferIter);

class CBufferPtr
{
public:
	CIndexData* m_indexPtr;
	uint64_t m_vectorNO;
	CBufferPtr():m_indexPtr(NULL),m_vectorNO(0){}
	CBufferPtr(const CIndexData* index,const uint64_t vectorno)
	{
		this->m_indexPtr=const_cast<CIndexData*>(index);
		this->m_vectorNO=vectorno;
	}
	CBufferPtr& operator=(const CBufferPtr& var)
	{
		this->m_indexPtr=var.m_indexPtr;
		this->m_vectorNO=var.m_vectorNO;
		return *this;
	}
};

typedef unordered_map<uint64_t,CBufferPtr>::iterator BufferIter;
typedef pair <uint64_t,CBufferPtr> Buffer_Pair;

class CBufferMgr : public unordered_map<uint64_t,CBufferPtr>
{
	friend class CBufferTest;
private:
	CConfigFile m_configFile;
	uint64_t m_clockHand;
	int64_t m_remain;
	vector<CBufferIter> m_iters;
	vector<uint64_t> m_freeVector;
	const bool EnoughMem() const;
	const Status AllocBuf(const CBufferID& filepageID,CBufferIter& iter,const CIndexData* index);
public:
	CBufferMgr(Status& status);
	~CBufferMgr();
	const size_t GetBufNum() const;
	const Status Initialize(CWordFile& wordFile);
	const Status AllocPage(CBufferID& filepageID,const CIndexData* index);
	const Status ReadPage(const CBufferID& filepageID, CIndexData*& indexData);
	const Status UnPinPage(const CBufferID& filepageID, const bool dirty);
	const Status FlushPage(const uint64_t& bufferID);
	const Status FlushAll(void);
	const string GetWordFileName(void)
	{
		return this->m_configFile.GetWordFileName();
	}
};

class CBufferTest
{
public:
	void RunTest(void) const;
};

#endif
