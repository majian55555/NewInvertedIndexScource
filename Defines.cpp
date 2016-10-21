#include <memory>
#include <cassert>
#include "Defines.h"

/*string ws2s(wstring& ws)
{
	string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest,0,_Dsize);
	wcstombs(_Dest,_Source,_Dsize);
	string result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}
wstring s2ws( const std::string& s )
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	std::wstring result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, "C");
	return result;
}*/
///////////////////////////////////Implementation of Class CByteUInt//////////////////////////////////////
const unsigned short CByteUInt::MaxNum() const
{
	unsigned short result=1;
	return (result<<ByteLen)-1;
}
CByteUInt::CByteUInt(uint64_t num)
{
	memcpy(&this->m_data,&num,1);
}
CByteUInt::CByteUInt(uint32_t num)
{
	memcpy(&this->m_data,&num,1);
}
CByteUInt::CByteUInt(unsigned short num)
{
	memcpy(&this->m_data,&num,1);
}
CByteUInt::CByteUInt(unsigned char data)
{
	memcpy(&this->m_data,&data,1);
}
CByteUInt::CByteUInt(const char* data)
{
	memcpy(&this->m_data,data,1);
}
CByteUInt::CByteUInt(const CByteUInt& data)
{
	*this=data;
}
const uint64_t CByteUInt::GetNum64() const
{
	uint64_t result=0;
	memcpy(&result,&this->m_data,1);
	return result;
}
const uint32_t CByteUInt::GetNum32() const
{
	uint32_t result=0;
	memcpy(&result,&this->m_data,1);
	return result;
}
const unsigned short CByteUInt::GetNum16() const
{
	unsigned short result=0;
	memcpy(&result,&this->m_data,1);
	return result;
}
const bool CByteUInt::IsZero() const
{
	if(this->m_data==0)
		return true;
	return false;
}
CByteUInt& CByteUInt::operator++()
{
	this->m_data+=1;
	return *this;
}
CByteUInt& CByteUInt::operator--()
{
	this->m_data-=1;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////Implementation of Class CVarWord////////////////////////////////////////
CVarWord::CVarWord(const CByteUInt& wordLen,const char* pointer):m_data(NULL)
{
	this->m_data=new char[wordLen.GetNum16()+1];
	memcpy(this->m_data,&wordLen,SizeOfCByteUInt);
	memcpy(this->m_data+1,pointer,wordLen.GetNum16());
}
CVarWord::CVarWord(const char* pointer):m_data(NULL)
{
	CByteUInt tmpBUI(pointer);
	this->m_data=new char[tmpBUI.GetNum16()+1];
	memcpy(this->m_data,pointer,tmpBUI.GetNum16()+1);
}
const unsigned short CVarWord::GetWordLen(void) const
{
	CByteUInt tmpBUI(this->m_data);
	return tmpBUI.GetNum16();
}
const char* CVarWord::GetWord(void) const
{
	return this->m_data+SizeOfCByteUInt;
}
const unsigned short CVarWord::GetTotalLen(void) const
{
	return this->GetWordLen()+SizeOfCByteUInt;
}
const char* CVarWord::GetTotalData(void) const
{
	return this->m_data;
}
void CVarWord::ShowWord() const
{
	string tmpstr(this->m_data+1,this->GetWordLen());
	cout<<tmpstr.c_str()<<endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Implementation of Class CBufferID//////////////////////////////////////
CBufferID::CBufferID(const unsigned short& fileID,const uint64_t& pageNo)
{
	CByteUInt tmpUInt(fileID);
	this->m_fileID=tmpUInt;
	memset(this->m_pageNo,0,SizeOfCBufferID-SizeOfCByteUInt);
	memcpy(this->m_pageNo,&pageNo,SizeOfCBufferID-SizeOfCByteUInt);
}
CBufferID::CBufferID(const CByteUInt& fileID,const uint64_t& pageNo)
{
	this->m_fileID=fileID;
	memset(this->m_pageNo,0,SizeOfCBufferID-SizeOfCByteUInt);
	memcpy(this->m_pageNo,&pageNo,SizeOfCBufferID-SizeOfCByteUInt);
}
CBufferID::CBufferID(const uint64_t& filepageNo)
{
	memcpy(this,&filepageNo,SizeOfCBufferID);
}
const CByteUInt CBufferID::GetFileID(void) const
{
	return this->m_fileID;
}
const uint64_t CBufferID::GetPageNo(void) const
{
	uint64_t result=0;
	memcpy(&result,this->m_pageNo,7);
	return result;
}
const uint64_t CBufferID::GetInt64(void) const
{
	return *(uint64_t*)this;
}
void CBufferID::SetBufferID(const uint64_t& bufferid)
{
	memcpy(this,&bufferid,SizeOfCBufferID);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CDefinesTest/////////////////////////////////////
void CDefinesTest::RunTest() const
{
	cout<<"Size of CByteUInt:"<<SizeOfCByteUInt<<endl;
	string str("ÄãÃÇºÃ");
	CByteUInt strlen(str.length());
	char* tmpchr=new char[str.length()+1];
	memcpy(tmpchr,&strlen,SizeOfCByteUInt);
	memcpy(tmpchr+SizeOfCByteUInt,str.c_str(),strlen.GetNum16());
	CVarWord vchar(tmpchr);
	delete [] tmpchr;
	assert(strncmp(vchar.GetWord(),str.c_str(),str.length())==0);
	cout<<"Test CVarWord successfully."<<endl;
	uint64_t tmp64=213;
	CByteUInt bui(tmp64);
	assert(255==bui.MaxNum());
	assert(213==bui.GetNum64());
	CByteUInt bui2(bui);
	assert(213==bui2.GetNum64());
	assert(false==bui2.IsZero());
	CBufferID tmpBufferID(255,240000);
	assert(255==tmpBufferID.GetFileID().GetNum32());
	assert(240000==tmpBufferID.GetPageNo());
	cout<<"Test CByteUInt successfully."<<endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
