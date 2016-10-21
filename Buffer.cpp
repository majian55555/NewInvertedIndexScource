// chose 5 suanfa 
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>
#include "Buffer.h"
///////////////////////////////////Implementation of Class CBufferMgr//////////////////////////////////////
CBufferMgr::CBufferMgr(Status& status)
{
	this->m_clockHand=0;
	this->clear();
	this->m_iters.clear();
	this->m_freeVector.clear();
	if((status=this->m_configFile.ReadConfigFile())!=OK)
		return;
	status=this->m_configFile.OpenInvertedFiles();
	/*FILE *stream=NULL;
	char line[MAXNAMESIZE];
	uint32_t memtotal=0,memfree=0;
	stream=fopen("/proc/meminfo", "r" );
	if( NULL==stream )
	{
		cout<<"Can't open /proc/meminfo file:";
		status = FILENOTEXIST;
		return;
	}
	while(fgets( line, MAXNAMESIZE, stream ))
	{
		if (!line[0]) continue;
		if (line[0] == '#') continue;
		char tmp[MAXNAMESIZE];
		char* begin=line;
		int i=0;
		for(;begin < line+strlen(line);i++,begin++)
		{
			if(*begin == ' ')
			{
				i--;
				continue;
			}
			if(*begin == '\n')
				break;
			else
				tmp[i]=*begin;
		}
		//tmp[i-2]='\0';
		if(strncmp("MemTotal:",tmp,9)==0)
		{
			memtotal=atoi(&tmp[9]);
			continue;
		}
		else if(strncmp("MemFree:",tmp,8)==0)
		{
			memfree=atoi(&tmp[8]);
			break;
		}
	}
	fclose(stream);
	double memload=(double)(memtotal-memfree)/(double)memtotal;
	memload=(double)this->m_configFile.GetMemLoad()/100-memload;
	int64_t tmp=(int64_t)(memload*memtotal*1024);
	if(tmp>MinBufferNum)
		this->m_remain=tmp;
	else
		status=MEMNOTENOUGH;*/
	this->m_remain=(uint64_t)4*(uint64_t)1024*(uint64_t)1024*(uint64_t)1024;
}
CBufferMgr::~CBufferMgr()
{
	BufferIter iter=this->begin();
	for(;iter!=this->end();iter++)
		delete (iter->second).m_indexPtr;
	this->m_iters.clear();
	this->clear();
	this->m_configFile.CloseInvertedFiles();
}
const bool CBufferMgr::EnoughMem() const
{
	return this->m_remain>0;
}
const Status CBufferMgr::AllocBuf(const CBufferID& filepageID,CBufferIter& iter,const CIndexData* index)
{
	Status status=OK;
	uint32_t allpin=this->size(),startat=0;
	while(!this->EnoughMem())
	{
		if(this->m_clockHand>=this->m_iters.size())
			this->m_clockHand=0;
		vector<CBufferIter>::iterator vectiter=this->m_iters.begin()+this->m_clockHand++;
		if(!vectiter->GetValid())
			continue;
		if(vectiter->GetPinCnt())
		{
			if(allpin>=startat++) return ALLPAGESPINED;
			continue;
		}
		if(vectiter->GetRefbit())
		{
			vectiter->SetRefbit(false);
			continue;
		}
		if(vectiter->GetDirty())
		{
			if((status=this->FlushPage(vectiter->GetBufferID()))!=OK)
				return status;
		}
		BufferIter bufferiter=this->find(vectiter->GetBufferID());
		unsigned int tmp=(bufferiter->second).m_indexPtr->GetLength();
		delete (bufferiter->second).m_indexPtr;
		this->erase(bufferiter);
		vectiter->SetValid(false);
		this->m_freeVector.push_back(vectiter-this->m_iters.begin());
		this->m_remain+=tmp;
	}
	if(this->m_freeVector.empty())
	{
		CBufferPtr bufferPtr(index,this->m_iters.size());
		this->insert(Buffer_Pair(filepageID.GetInt64(),bufferPtr));
		iter.m_bufferID=filepageID.GetInt64();
		iter.AddPinCnt();
		iter.SetRefbit(true);
		this->m_iters.push_back(iter);
		this->m_remain-=(index->GetLength()+CBufferIterSize+sizeof(uint64_t));
	}
	else
	{
		uint64_t vectorNo=*(this->m_freeVector.begin());
		this->m_freeVector.erase(this->m_freeVector.begin());
		vector<CBufferIter>::iterator vectiter=this->m_iters.begin()+vectorNo;
		vectiter->m_bufferID=filepageID.GetInt64();
		vectiter->SetValid(true);
		vectiter->AddPinCnt();
		vectiter->SetRefbit(true);
		CBufferPtr bufferPtr(index,vectorNo);
		this->insert(Buffer_Pair(filepageID.GetInt64(),bufferPtr));
		this->m_remain-=index->GetLength();
	}
	return OK;
}
const size_t CBufferMgr::GetBufNum() const
{
	return this->m_iters.size();
}
const Status CBufferMgr::AllocPage(CBufferID& filepageID,const CIndexData* index)
{
	Status status=OK;
	unsigned short fileID=this->m_configFile.FindFitInvertedFile(index->GetLength());
	InvertedFile_Iter invertedFile_iter=this->m_configFile.find(fileID);
	uint64_t newPageNo=0;
	if((status=invertedFile_iter->second.AllocateNewPages(newPageNo,index))!=OK)
		return status;
	CBufferID bufferID(fileID,newPageNo);
	filepageID.SetBufferID(bufferID.GetInt64());
	CBufferIter iter;
	if((status=this->AllocBuf(filepageID,iter,index))!=OK)
		return status;
	return OK;
}
const Status CBufferMgr::ReadPage(const CBufferID& filepageID, CIndexData*& indexData)
{
	BufferIter iter=this->find(filepageID.GetInt64());
	indexData=new CIndexData();
	if(this->end()==iter)	// Can't find it in hash map.
	{
		Status status=OK;
		unsigned short fileID=filepageID.GetFileID().GetNum16();
		InvertedFile_Iter invertedFileIter=this->m_configFile.find(fileID);
		if(invertedFileIter==this->m_configFile.end())
			return BADFILEID;
		CBufferIter iter;
		if((status=invertedFileIter->second.ReadIndex(filepageID.GetPageNo(),indexData))!=OK)
			return status;
		if((status=this->AllocBuf(filepageID,iter,indexData))!=OK)
			return status;
	}
	else	// Find it in hash map.
	{
		vector<CBufferIter>::iterator cbufferIter=this->m_iters.begin()+(iter->second).m_vectorNO;
		cbufferIter->AddPinCnt();
		cbufferIter->SetRefbit(true);
		indexData=(iter->second).m_indexPtr;
	}
	return OK;
}
const Status CBufferMgr::UnPinPage(const CBufferID& filepageID, const bool dirty)
{
	BufferIter bufferiter=this->find(filepageID.GetInt64());
	if(bufferiter==this->end())
		return HASHNOTFOUND;
	vector<CBufferIter>::iterator iter=this->m_iters.begin()+(bufferiter->second).m_vectorNO;
	if(!iter->GetValid())
		return BUFFERNOTVALID;
	if(iter->GetPinCnt()<=0)
		return PAGENOTPINED;
	iter->UnPin();
	if(dirty)
		iter->SetDirty();
	return OK;
}
const Status CBufferMgr::Initialize(CWordFile& wordFile)
{
	Status status=OK;
	unordered_map<string,CWordInfo>::iterator iter=wordFile.begin();
	cout<<"Using "<<this->m_remain<<" bytes for buffer."<<endl;
	for(;this->EnoughMem()&&iter!=wordFile.end();++iter)
	{
		//cout<<(iter->first).c_str()<<endl;
		CBufferID bufferID((iter->second).m_fileNo,(iter->second).m_pageNo);
		CIndexData* index=NULL;
		if((status=this->ReadPage(bufferID, index))!=OK)
			return status;
		if((status=this->UnPinPage(bufferID, false))!=OK)
			return status;
	}
	return OK;
}
const Status CBufferMgr::FlushPage(const uint64_t& bufferID)
{
	CBufferID bufID(bufferID);
	InvertedFile_Iter invertedFileIter=this->m_configFile.find(bufID.GetFileID().GetNum16());
	BufferIter bufferiter=this->find(bufferID);
	return invertedFileIter->second.WriteIndex(bufID.GetPageNo(),(bufferiter->second).m_indexPtr);
}
const Status CBufferMgr::FlushAll(void)
{
	Status status=OK;
	InvertedFile_Iter invertedFile_Iter=this->m_configFile.begin();
	for(;invertedFile_Iter!=this->m_configFile.end();invertedFile_Iter++)
	{
		if((status=(invertedFile_Iter->second).FlushHeadInfo())!=OK)
			return status;
	}
	vector<CBufferIter>::iterator iter=this->m_iters.begin();
	for(;iter!=this->m_iters.end();iter++)
	{
		if(iter->GetPinCnt())
			return PAGEPINED;
		if(iter->GetDirty())
		{
			if((status=this->FlushPage(iter->GetBufferID()))!=OK)
				return status;
		}
	}
	return OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Implementation of Class CBufferTest/////////////////////////////////////
const uint64_t TestNum=100;
void CBufferTest::RunTest(void) const
{
	Error error;
	Status status=OK;
	CBufferMgr* bufferMgr=new CBufferMgr(status);
	error.MyAssertOK(status);
	/*assert(bufferMgr->EnoughMem());
	assert(0==bufferMgr->GetBufNum());
	CBufferID filepageID(0,0);
	char tmpchr[50];
	CIndexData index(50,tmpchr);
	error.MyAssertOK(bufferMgr->AllocPage(filepageID,&index));
	assert(1==filepageID.GetPageNo());
	assert(1==bufferMgr->m_iters.size());
	error.MyAssertOK(bufferMgr->UnPinPage(filepageID,false));*/
	/*uint32_t i=0;
	for (i = 0; i < TestNum; i++) 
	{
		CBufferID filepageID(0,0);
		string tmpstr;
		tmpstr.assign("test.1 Page:");
		char tmpchr[10];
		sprintf(tmpchr,"%u",i);
		tmpstr.append(tmpchr);
		CIndexData* index=new CIndexData((uint32_t)tmpstr.length(),tmpstr.c_str());
		error.MyAssertOK(bufferMgr->AllocPage(filepageID,index));
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID,false));
	}
	cout<<"Allocate Inverted Files' pages successfully."<<endl;
	cout <<"Test1 passed!"<<endl<<endl;
    cout << "Reading pages back..." << endl;
    for (i = 0; i < TestNum; i++)
	{
		CBufferID filepageID(0,i+1);
		CIndexData* index=NULL;
		error.MyAssertOK(bufferMgr->ReadPage(filepageID, index));
		string tmpstr;
		tmpstr.assign("test.1 Page:");
		char tmpchr[10];
		sprintf(tmpchr,"%u",i);
		tmpstr.append(tmpchr);
		assert(memcmp(index->GetData(), tmpstr.c_str(),(uint32_t)tmpstr.length()) == 0);
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID,false));
    }
    cout<< "Test2 passed!"<<endl<<endl;
	cout << "Writing and reading back multiple files..." << endl;
    for (i = 0; i < TestNum; i++) 
	{
		CBufferID filepageID1(0,0),filepageID2(0,0);
		string tmpstr;
		tmpstr.assign("test.3 Page:");
		char tmpchr[10];
		sprintf(tmpchr,"%u",i);
		tmpstr.append(tmpchr);
		char tmp1[4096],tmp2[8192];
		memset(tmp1,0,4096);
		memset(tmp2,0,8192);
		strcpy(tmp1,tmpstr.c_str());
		strcpy(tmp2,tmpstr.c_str());
		CIndexData *index1=new CIndexData(4096,tmp1);
		CIndexData *index2=new CIndexData(8192,tmp2);
		error.MyAssertOK(bufferMgr->AllocPage(filepageID1,index1));
		error.MyAssertOK(bufferMgr->AllocPage(filepageID2,index2));
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID1,false));
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID2,false));
	}
	for (i = 0; i < TestNum; i++)
	{
		CBufferID filepageID1(1,i+1),filepageID2(2,i+1);
		CIndexData *index1=NULL,*index2=NULL;
		error.MyAssertOK(bufferMgr->ReadPage(filepageID1, index1));
		error.MyAssertOK(bufferMgr->ReadPage(filepageID2, index2));
		string tmpstr;
		tmpstr.assign("test.3 Page:");
		char tmpchr[10];
		sprintf(tmpchr,"%u",i);
		tmpstr.append(tmpchr);
		char tmp1[512],tmp2[1024];
		strcpy(tmp1,tmpstr.c_str());
		strcpy(tmp2,tmpstr.c_str());
		assert(memcmp(index1->GetData(), tmp1,(uint32_t)strlen(tmp1)) == 0);
		assert(memcmp(index1->GetData(), tmp1,(uint32_t)strlen(tmp1)) == 0);
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID1,false));
		error.MyAssertOK(bufferMgr->UnPinPage(filepageID2,false));
    }
	cout<< "Test3 passed!"<<endl<<endl;*/
	//
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem(bufferMgr->m_configFile.GetWordFileName()));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	cout<<"Initializing buffer ....."<<endl;
	error.MyAssertOK(bufferMgr->Initialize(wordFile));
	cout<<"Initialize buffer successfully."<<endl;
	error.MyAssertOK(bufferMgr->FlushAll());
	cout<<"Flush Inverted Files successfully."<<endl;
	cout<<"Size of hash map:"<<bufferMgr->size()<<endl;
	delete bufferMgr;
	cout<<"Delete buffer manager successfully."<<endl;
	return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
