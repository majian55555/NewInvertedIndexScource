#include "Pages.h"
#include <cassert>
///////////////////////////////////Implementation of Class CPage.////////////////////////////////////////
CPage::CPage(const uint32_t pageSize):m_pageSize(pageSize)
{
	//this->m_data=new char[pageSize];
	//assert(this->m_data!=NULL);
	assert(0==posix_memalign((void**)&this->m_data, getpagesize(), pageSize));
	memset(this->m_data,0,pageSize);
}
const uint64_t CPage::GetPageNo(void) const
{
	return *(uint64_t*)this->m_data;
}
const uint64_t CPage::GetNextPageNo(void) const
{
	return *(((uint64_t*)this->m_data)+1);
}
const uint32_t CPage::GetDateLength(void) const
{
	uint32_t result=0;
	memcpy(&result,this->m_data+2*sizeof(uint64_t),sizeof(result));
	return result;
}
void CPage::SetPageNo(uint64_t pageNo)
{
	memcpy(this->m_data,&pageNo,sizeof(uint64_t));
}
void CPage::SetNextPageNo(uint64_t nextPageNo)
{
	memcpy(this->m_data+sizeof(uint64_t),&nextPageNo,sizeof(uint64_t));
}
void CPage::SetDateLength(uint32_t dataLen)
{
	memcpy(this->m_data+2*sizeof(uint64_t),&dataLen,sizeof(dataLen));
}
const char* CPage::GetData() const
{
	return this->m_data+PageInfoSize;
}
void CPage::SetData(const char* pointer,const uint32_t& length)
{
	memcpy(this->m_data+PageInfoSize,pointer,length);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CIndexData.//////////////////////////////////////
CIndexData::CIndexData(const uint32_t len,const char* pointer)
{
	this->m_length=len;
	this->m_data=new char[len];
	assert(this->m_data!=NULL);
	memcpy(this->m_data,pointer,len);
}
CIndexData::~CIndexData()
{
	if(this->m_data)
			delete [] this->m_data;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CPagesTest.//////////////////////////////////////
void CPagesTest::RunTest(void) const
{
	CPage page(512);
	page.SetPageNo(0);
	page.SetNextPageNo(0);
	char chrary[400];
	memset(chrary,0,400);
	assert(0==memcmp(page.GetData(),chrary,400));
	memset(chrary,1,400);
	page.SetData(chrary,400);
	assert(0==page.GetPageNo());
	assert(0==page.GetNextPageNo());
	assert(0==memcmp(page.GetData(),chrary,400));
	cout<<"Read page data successfully."<<endl;
	return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
