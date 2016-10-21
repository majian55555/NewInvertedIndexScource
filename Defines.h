#ifndef BYTEINT_H
#define BYTEINT_H

#include <stdint.h>
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

const unsigned int MAXNAMESIZE=255;	// Maximum file name length.
const unsigned short ByteLen=8;
class CByteUInt	// This class is a type of unsigned int just using one byte(can represent 0-255).
{
private:
	unsigned char m_data;
public:
	CByteUInt(){this->m_data=0;}
	CByteUInt(uint64_t num);
	CByteUInt(uint32_t num);
	CByteUInt(unsigned short num);
	CByteUInt(unsigned char data);
	CByteUInt(const char* data);
	CByteUInt(const CByteUInt& data);
	CByteUInt& operator=(const CByteUInt& var)
	{
		this->m_data=var.m_data;
		return *this;
	}
	const uint64_t GetNum64() const;
	const uint32_t GetNum32() const;
	const unsigned short GetNum16() const;
	const unsigned short MaxNum() const;
	const bool IsZero() const;
	CByteUInt& operator++();
	CByteUInt& operator--(); 
};

const unsigned short SizeOfCByteUInt=sizeof(CByteUInt);

class CVarWord	// This class is a type of char string. The length is from 0 to 255.
{
private:
	char* m_data;
public:
	CVarWord(const CByteUInt& wordLen,const char* pointer);
	CVarWord(const char* pointer);
	CVarWord& operator=(const CVarWord& var)
	{
		this->m_data=var.m_data;
		return *this;
	}
	~CVarWord()
	{
		if(this->m_data)
			delete [] this->m_data;
	}
	const unsigned short GetWordLen(void) const;
	const char* GetWord(void) const;
	const unsigned short GetTotalLen(void) const;
	const char* GetTotalData(void) const;
	void ShowWord() const;
};

//const unsigned short SizeOfWChar=sizeof(wchar_t);
const unsigned short CWordInfoSize=sizeof(uint32_t)*2+sizeof(uint64_t)+sizeof(CByteUInt);

class CWordInfo	// This class contains word's information.
{
public:
	uint64_t m_pageNo;
	uint32_t m_urlCount;
	uint32_t m_totalCount;
	CByteUInt m_fileNo;
	CWordInfo(const char* pointer)
	{
		memcpy(this,pointer,CWordInfoSize);
	}
	CWordInfo():m_fileNo((unsigned char)0)
	{
		this->m_pageNo=0;
		this->m_urlCount=0;
		this->m_totalCount=0;
	}
	CWordInfo& operator=(const CWordInfo& var)
	{
		this->m_pageNo=var.m_pageNo;
		this->m_urlCount=var.m_urlCount;
		this->m_fileNo=var.m_fileNo;
		this->m_totalCount=var.m_totalCount;
		return *this;
	}
};

class CBufferID	// This class represents the key of buffer's hash_map.
{
private:
	CByteUInt m_fileID;
	char m_pageNo[7];
public:
	CBufferID(const unsigned short& fileID,const uint64_t& pageNo);
	CBufferID(const CByteUInt& fileID,const uint64_t& pageNo);
	CBufferID(const uint64_t& filepageNo);
	const CByteUInt GetFileID(void) const;
	const uint64_t GetPageNo(void) const;
	const uint64_t GetInt64(void) const;
	void SetBufferID(const uint64_t& bufferid);
};

const unsigned short SizeOfCBufferID=sizeof(CBufferID);

class CDefinesTest
{
public:
	void RunTest(void) const;
};

#endif
