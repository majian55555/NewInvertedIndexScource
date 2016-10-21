#include "FileOp.h"
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>

///////////////////////////////////Implementation of Class CFileOp//////////////////////////////////////
CFileOp::CFileOp(void)
{
	this->m_fileHandle=0;
}
CFileOp::~CFileOp()
{
	if(this->m_fileHandle>0)
		this->CloseFile();
}
const Status CFileOp::GetFileSize(int64_t& filesize) const
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	filesize=::lseek(this->m_fileHandle,0,SEEK_END);
	if(filesize==-1)
		return SYSERR;
	return OK;
}
const Status CFileOp::CreateFile(const string& fileName)
{
	if(fileName.empty()||fileName.length()>MAXNAMESIZE)
		return BADFILENAME;
	this->m_fileHandle=::open(fileName.c_str(),O_CREAT | O_EXCL | O_WRONLY, 0666);
	if(-1== this->m_fileHandle)
		return SYSERR;
	Status status;
	if((status=this->CloseFile())!=0)
		return status;
	return OK;
}
const Status CFileOp::DeleteFile(const string& fileName)
{
	if(fileName.empty()||fileName.length()>MAXNAMESIZE)
		return BADFILENAME;
	if(this->m_fileHandle>0)
		return FILEISOPEN;
	if(::remove(fileName.c_str())==-1)
		return SYSERR;
	this->m_fileHandle=0;
	return OK;
}
const Status CFileOp::OpenFile(const string& fileName)
{
	if(fileName.empty()||fileName.length()>MAXNAMESIZE)
		return BADFILENAME;
	this->m_fileHandle=::open(fileName.c_str(),O_RDWR|O_DIRECT|O_LARGEFILE);
	if(-1==this->m_fileHandle)
		return SYSERR;
	return OK;
}
const Status CFileOp::OpenFile2(const string& fileName)
{
	if(fileName.empty()||fileName.length()>MAXNAMESIZE)
		return BADFILENAME;
	this->m_fileHandle=::open(fileName.c_str(),O_RDWR|O_LARGEFILE);
	if(-1==this->m_fileHandle)
		return SYSERR;
	return OK;
}
const Status CFileOp::CloseFile()
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	if(::close(this->m_fileHandle)==-1)
		return SYSERR;
	this->m_fileHandle=0;
	return OK;
}
const Status CFileOp::ReadFile(void* buffer,const int64_t& offset,const int32_t length) const
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	if(-1==lseek64(this->m_fileHandle,offset,SEEK_SET))
		return SYSERR;
	if(length!=::read(this->m_fileHandle,buffer,length))
		return SYSERR;
	//posix_fadvise(this->m_fileHandle, 0, 0, POSIX_FADV_DONTNEED);
	return OK;
}
const Status CFileOp::WriteFile(const void* buffer,const int64_t& offset,const int32_t length) const
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	if(-1==lseek64(this->m_fileHandle,offset,SEEK_SET))
		return SYSERR;
	if(length!=::write(this->m_fileHandle,buffer,length))
		return SYSERR;
	//posix_fadvise(this->m_fileHandle, 0, 0, POSIX_FADV_DONTNEED);
	//if(-1==fsync(this->m_fileHandle))
		//return SYSERR;
	return OK;
}
const Status CFileOp::ExtendFile(const int32_t size,const void* buffer) const
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	if(-1==lseek64(this->m_fileHandle,0,SEEK_END))
		return SYSERR;
	if(buffer)
	{
		if(size!=::write(this->m_fileHandle,buffer,size))
			return SYSERR;
	}
	else
	{
		//char *tmpchar=new char[size];
		char *tmpchar=NULL;
		assert(0==posix_memalign((void**)&tmpchar, getpagesize(), size));
		memset(tmpchar,0,size);
		if(size!=::write(this->m_fileHandle,(void*)tmpchar,size))
		{
			//delete [] tmpchar;
			free(tmpchar);
			return SYSERR;
		}
		//delete [] tmpchar;
		free(tmpchar);
	}
	//posix_fadvise(this->m_fileHandle, 0, 0, POSIX_FADV_DONTNEED);
	//if(-1==fsync(this->m_fileHandle))
		//return SYSERR;
	//if(-1==fdatasync(this->m_fileHandle))
		//return SYSERR;
	return OK;
}
const Status CFileOp::TruncFile(const int32_t size,const string& fileName)
{
	if(this->m_fileHandle<=0)
		return FILENOTOPEN;
	if(::close(this->m_fileHandle)==-1)
		return SYSERR;
	this->m_fileHandle=::open(fileName.c_str(),O_RDWR|O_TRUNC);
	if(-1==this->m_fileHandle)
		return SYSERR;
	//char *tmpchar=new char[size];
	char *tmpchar=NULL;
	assert(0==posix_memalign((void**)&tmpchar, getpagesize(), size));
	memset(tmpchar,0,size);
	if(size!=::write(this->m_fileHandle,tmpchar,size))
	{
		//delete [] tmpchar;
		free(tmpchar);
		return SYSERR;
	}
	//delete [] tmpchar;
	free(tmpchar);
	//posix_fadvise(this->m_fileHandle, 0, 0, POSIX_FADV_DONTNEED);
	return OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////Implementation of Class CInvertedIndexFile///////////////////////////////////
CInvertedFile::CInvertedFile(void)
{
	this->m_fileName.clear();
	this->m_headPage.m_nextFreePage=0;
	this->m_headPage.m_pageCnt=0;
	this->m_headPage.m_pageSize=0;
}
CInvertedFile::CInvertedFile(const string& fileName,Status& status)
{
	if((status=this->m_fileOp.OpenFile(fileName))!=OK)
		return;
	//if((status=this->m_fileOp.ReadFile((void*)&this->m_headPage,0,HeadPageSize))!=OK)
		//return;
	char* tmp=NULL;
	assert(0==posix_memalign((void**)&tmp, getpagesize(), getpagesize()));
	if((status=this->m_fileOp.ReadFile((void*)tmp,0,getpagesize()))!=OK)
	{
		free(tmp);
		return;
	}
	memcpy(&this->m_headPage,tmp,HeadPageSize);
	free(tmp);
}
const Status CInvertedFile::OpenInvertedFile(void)
{
	Status status=OK;
	if((status=this->m_fileOp.OpenFile(this->GetFileName()))!=OK)
		return status;
	char* tmp=NULL;
	assert(0==posix_memalign((void**)&tmp, getpagesize(), 512));
	if((status=this->m_fileOp.ReadFile((void*)tmp,0,512))!=OK)
		return status;
	memcpy(&this->m_headPage,tmp,HeadPageSize);
	free(tmp);
	return status;
}
const Status CInvertedFile::CloseInvertedFile(void)
{
	return this->m_fileOp.CloseFile();
}
const string CInvertedFile::GetFileName(void) const
{
	return this->m_fileName;
}
const uint32_t CInvertedFile::GetPageSize(void) const
{
	return this->m_headPage.m_pageSize;
}
const uint64_t CInvertedFile::GetPageCnt(void) const
{
	return this->m_headPage.m_pageCnt;
}
const Status CInvertedFile::CreateInvertedFile(const string& fileName,const uint32_t pageSize)
{
	Status status;
	if((status=this->m_fileOp.CreateFile(fileName))!=OK)
		return status;
	if((status=this->m_fileOp.OpenFile(fileName))!=OK)
		return status;
	this->m_fileName=fileName;
	this->m_headPage.m_nextFreePage=0;
	this->m_headPage.m_pageCnt=1;
	this->m_headPage.m_pageSize=pageSize;
	//char* tmp=new char[pageSize];
	char* tmp=NULL;
	assert(0==posix_memalign((void**)&tmp, getpagesize(), pageSize));
	memset(tmp,0,pageSize);
	memcpy(tmp,&this->m_headPage,HeadPageSize);
	if((status=this->m_fileOp.WriteFile((void*)tmp,0,pageSize))!=OK)
	{
		delete [] tmp;
		return status;
	}
	delete [] tmp;
	if((status=this->m_fileOp.CloseFile())!=OK)
		return status;
	return OK;
}
const Status CInvertedFile::DeleteInvertedFile(const string& fileName)
{
	return this->m_fileOp.DeleteFile(fileName);
}
const Status CInvertedFile::ReadIndex(const uint64_t pageNo,CIndexData* index) const
{
	Status status=OK;
	uint64_t tmpPageNo=pageNo;
	CPage tmpPage(this->GetPageSize());
	uint64_t offset=(uint64_t)tmpPageNo*(uint64_t)this->m_headPage.m_pageSize;
	if((status=this->m_fileOp.ReadFile(const_cast<char*>(tmpPage.GetTotalData()),offset,this->m_headPage.m_pageSize))!=OK)
		return status;
	index->SetLength(tmpPage.GetDateLength());
	uint32_t pagecnt=index->GetLength()/(this->GetPageSize()-PageInfoSize)+1;
	tmpPageNo=tmpPage.GetNextPageNo();
	uint32_t i=0,remain=index->GetLength();
	if((this->GetPageSize()-PageInfoSize)>=remain)
		memcpy((void*)index->GetData(),tmpPage.GetData(),remain);
	else
		memcpy((void*)index->GetData(),tmpPage.GetData(),this->GetPageSize()-PageInfoSize);
	remain-=(this->GetPageSize()-PageInfoSize);
	for(i=1;i<pagecnt;i++)
	{
		uint64_t offset=(uint64_t)tmpPageNo*(uint64_t)this->m_headPage.m_pageSize;
		if((status=this->m_fileOp.ReadFile(const_cast<char*>(tmpPage.GetTotalData()),offset,this->m_headPage.m_pageSize))!=OK)
			return status;
		if((this->GetPageSize()-PageInfoSize)>=remain)
			memcpy((void*)(index->GetData()+i*(this->GetPageSize()-PageInfoSize)),tmpPage.GetData(),remain);
		else
			memcpy((void*)(index->GetData()+i*(this->GetPageSize()-PageInfoSize)),tmpPage.GetData(),this->GetPageSize()-PageInfoSize);
		remain-=(this->GetPageSize()-PageInfoSize);
	}
	return OK;
}
const Status CInvertedFile::WriteIndex(const uint64_t pageNo,const CIndexData* index)
{
	Status status=OK;
	uint32_t pagecnt=index->GetLength()/(this->GetPageSize()-PageInfoSize)+1,i=0,remain=index->GetLength();
	uint64_t nextPageNo=pageNo;
	CPage* pages=new CPage[pagecnt];
	for(;i<pagecnt;i++)
	{
		pages[i].SetPageSize(this->GetPageSize());
		if(nextPageNo)
		{
			if((status=this->m_fileOp.ReadFile(const_cast<char*>(pages[i].GetTotalData()),nextPageNo*this->GetPageSize(),this->GetPageSize()))!=OK)
				return status;
			nextPageNo=pages[i].GetNextPageNo();
		}
		else
		{
			if(this->m_headPage.m_nextFreePage<=0)
				pages[i].SetPageNo(this->m_headPage.m_pageCnt++);
			else
			{
				pages[i].SetPageNo(this->m_headPage.m_nextFreePage);
				CPage newpage(this->GetPageSize());
				if((status=this->m_fileOp.ReadFile(const_cast<char*>(newpage.GetTotalData()),pages[i].GetPageNo()*this->GetPageSize(),this->GetPageSize()))!=OK)
				{
					delete [] pages;
					return status;
				}
				this->m_headPage.m_nextFreePage=newpage.GetNextPageNo();
			}
			nextPageNo=0;
		}
		if((this->GetPageSize()-PageInfoSize)>=remain)
			pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),remain);
		else
			pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),(this->GetPageSize()-PageInfoSize));
	}
	for(i=0;i<pagecnt;i++)
	{
		if(i+1==pagecnt)
		{
			pages[i].SetNextPageNo(0);
			break;
		}
		pages[i].SetNextPageNo(pages[i+1].GetPageNo());
	}
	for(i=0;i<pagecnt;i++)
	{
		if((status=this->m_fileOp.WriteFile(pages[i].GetTotalData(),pages[i].GetPageNo()*this->GetPageSize(),this->GetPageSize()))!=OK)
		{
			delete [] pages;
			return status;
		}
	}
	if(nextPageNo)
	{
		if((status=this->DisposePages(nextPageNo))!=OK)
		{
			delete [] pages;
			return status;
		}
	}
	delete [] pages;
	return OK;
}
const Status CInvertedFile::AllocateNewPages(uint64_t& pageNo,const CIndexData* index)
{
	Status status=OK;
	uint32_t pagecnt=index->GetLength()/(this->GetPageSize()-PageInfoSize)+1;
	CPage* pages=new CPage[pagecnt];
	uint32_t i=0,remain=index->GetLength();
	for(;i<pagecnt;i++)
	{
		pages[i].SetPageSize(this->GetPageSize());
		if(this->m_headPage.m_nextFreePage<=0)
		{
			pages[i].SetPageNo(this->m_headPage.m_pageCnt++);
			if((this->GetPageSize()-PageInfoSize)>=remain)
				pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),remain);
			else
				pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),(this->GetPageSize()-PageInfoSize));
		}
		else
		{
			pages[i].SetPageNo(this->m_headPage.m_nextFreePage);
			if((this->GetPageSize()-PageInfoSize)>=remain)
				pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),remain);
			else
				pages[i].SetData(index->GetData()+i*(this->GetPageSize()-PageInfoSize),(this->GetPageSize()-PageInfoSize));
			CPage newpage(this->GetPageSize());
			if((status=this->m_fileOp.ReadFile(const_cast<char*>(newpage.GetTotalData()),pages[i].GetPageNo()*this->GetPageSize(),this->GetPageSize()))!=OK)
			{
				delete [] pages;
				return status;
			}
			this->m_headPage.m_nextFreePage=newpage.GetNextPageNo();
		}
		pages[i].SetDateLength(index->GetLength());
		remain-=(this->GetPageSize()-PageInfoSize);
	}
	for(i=0;i<pagecnt;i++)
	{
		if(i+1==pagecnt)
		{
			pages[i].SetNextPageNo(0);
			break;
		}
		pages[i].SetNextPageNo(pages[i+1].GetPageNo());
	}
	for(i=0;i<pagecnt;i++)
	{
		if((status=this->m_fileOp.ExtendFile(this->GetPageSize(),pages[i].GetTotalData()))!=OK)
		{
			delete [] pages;
			return status;
		}
	}
	pageNo=pages[0].GetPageNo();
	delete [] pages;
	return OK;
}
const Status CInvertedFile::DisposePages(const uint64_t pageNo)
{
	Status status=OK;
	CPage page(this->m_headPage.m_pageSize);
	uint64_t tmpPageNo=pageNo;
	do
	{	
		if((status=this->m_fileOp.ReadFile(const_cast<char*>(page.GetTotalData()),tmpPageNo*this->GetPageSize(),this->GetPageSize()))!=OK)
			return status;
		tmpPageNo=page.GetNextPageNo();
		memset(const_cast<char*>(page.GetData()),0,this->m_headPage.m_pageSize);
		page.SetPageNo(tmpPageNo);
		page.SetNextPageNo(this->m_headPage.m_nextFreePage);
		if((status=this->m_fileOp.WriteFile(page.GetTotalData(),tmpPageNo*this->GetPageSize(),this->GetPageSize()))!=OK)
			return status;
		this->m_headPage.m_nextFreePage=tmpPageNo;
	}while(tmpPageNo);
	return OK;
}
const Status CInvertedFile::FlushHeadInfo(void)
{
	if(!this->m_fileOp.IsOpen())
		return FILENOTOPEN;
	//return this->m_fileOp.WriteFile((void*)&this->m_headPage,0,HeadPageSize);
	char* tmp=NULL;
	assert(0==posix_memalign((void**)&tmp, getpagesize(), this->m_headPage.m_pageSize));
	memset(tmp,0,this->m_headPage.m_pageSize);
	memcpy(tmp,&this->m_headPage,HeadPageSize);
	Status status=OK;
	if((status=this->m_fileOp.ReadFile((void*)tmp,0,this->m_headPage.m_pageSize))!=OK)
		return status;
	free(tmp);
	return OK;
}
CInvertedFile::~CInvertedFile(void)
{
	if(this->m_fileOp.IsOpen())
	{
		this->FlushHeadInfo();
		this->CloseInvertedFile();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CWordFile.///////////////////////////////////////
CWordFile::CWordFile()
{
	this->clear();
	this->m_modified=false;
}
CWordFile::~CWordFile()
{
	this->clear();
}
const Status CWordFile::CreateWordFile(const string& wordfilename)
{
	return this->m_fileOp.CreateFile(wordfilename);
}
const Status CWordFile::DeleteWordFile(const string& wordfilename)
{
	return this->m_fileOp.DeleteFile(wordfilename);
}
const Status CWordFile::AddNewWord(const string& word,const CWordInfo& wordinfo)
{
	unordered_map<string,CWordInfo>::iterator iter=this->find(word);
	if(this->end()!=iter)
		return WORDEXIST;
	this->insert(WordInfo_Pair(word,wordinfo));
	this->m_modified=true;
	return OK;
}
const Status CWordFile::UpdateWord(const string& word,const CWordInfo& wordinfo)
{
	unordered_map<string,CWordInfo>::iterator iter=this->find(word);
	if(iter==this->end())
		return WORDEXIST;
	iter->second=wordinfo;
	this->m_modified=true;
	return OK;
}
const Status CWordFile::DeleteWord(const string& word)
{
	unordered_map<string,CWordInfo>::iterator iter=this->find(word);
	if(iter==this->end())
		return WORDEXIST;
	this->erase(iter);
	this->m_modified=true;
	return OK;
}
const Status CWordFile::ReadFileToMem(const string& wordfilename)
{
	this->clear();
	Status status=OK;
	if((status=this->m_fileOp.OpenFile2(wordfilename))!=OK)
		return status;
	int64_t filesize=0,readsize=0;
	if((status=this->m_fileOp.GetFileSize(filesize))!=OK)
		return status;
	if(-1==::lseek(this->m_fileOp.m_fileHandle,0,SEEK_SET))
		return SYSERR;
	while(filesize>readsize)
	{
		CByteUInt buint;
		if(SizeOfCByteUInt!=::read(this->m_fileOp.m_fileHandle,&buint,SizeOfCByteUInt))
			return SYSERR;
		readsize+=SizeOfCByteUInt;
		char* tmpchr=new char[buint.GetNum32()];
		if(buint.GetNum32()!=(uint32_t)::read(this->m_fileOp.m_fileHandle,tmpchr,buint.GetNum32()))
		{
			delete [] tmpchr;
			return SYSERR;
		}
		readsize+=buint.GetNum32();
		CVarWord vword(buint,tmpchr);
		delete [] tmpchr;
		CWordInfo wordinfo;
		if(CWordInfoSize!=::read(this->m_fileOp.m_fileHandle,&wordinfo,CWordInfoSize))
			return SYSERR;
		readsize+=CWordInfoSize;
		string wordstr(vword.GetWord(),vword.GetWordLen());
		//cout<<wordstr.c_str()<<endl;
		this->insert(WordInfo_Pair(wordstr,wordinfo));
	}
	if((status=this->m_fileOp.CloseFile())!=OK)
		return status;
	return OK;
}
const Status CWordFile::FlushMenToFile(const string& wordfilename)
{
	if(this->m_modified)
	{
		Status status=OK;
		this->m_fileOp.DeleteFile(wordfilename);
		if((status=this->m_fileOp.CreateFile(wordfilename))!=OK)
			return status;
		if((status=this->m_fileOp.OpenFile2(wordfilename))!=OK)
			return status;
		if(-1==::lseek(this->m_fileOp.m_fileHandle,0,SEEK_SET))
			return SYSERR;
		unordered_map<string,CWordInfo>::iterator iter=this->begin();
		for(;iter!=this->end();++iter)
		{
			CByteUInt buint((uint32_t)iter->first.length());
			CVarWord vword(buint,iter->first.c_str());
			if(vword.GetTotalLen()!=::write(this->m_fileOp.m_fileHandle,vword.GetTotalData(),vword.GetTotalLen()))
				return SYSERR;
			if(CWordInfoSize!=::write(this->m_fileOp.m_fileHandle,&iter->second,CWordInfoSize))
				return SYSERR;
		}
		if((status=this->m_fileOp.CloseFile())!=OK)
			return status;
	}
	return OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CConfigFile./////////////////////////////////////
CConfigFile::CConfigFile():m_wordFileName(),m_largestFile(0)
{
}
CConfigFile::~CConfigFile()
{
	this->clear();
}
const Status CConfigFile::ReadConfigFile(void)
{
	FILE *stream=NULL;
	char line[MAXNAMESIZE];
	stream=fopen( ConfigFileName.c_str(), "r" );
	if( NULL==stream )
	{
		cout<<"Can't open config file:";
		return FILENOTEXIST;
	}
	CByteUInt fileNo((unsigned short)0);
	while(fgets( line, MAXNAMESIZE, stream ))
	{
		if (!line[0]) continue;
		if (line[0] == '#') continue;
		char tmp[MAXNAMESIZE];
		char* begin=line;
		int i=0;
		for(;begin < line+strlen(line);i++,begin++)
		{
			if(*begin == '\r' || *begin == '\n' || *begin == ' ' || *begin == '\t')
				tmp[i]=0;
			else
				tmp[i]=*begin;
		}
		tmp[i]='\0';
		if(strncmp("WordFilePath",tmp,12)==0)
		{
			this->m_wordFileName.assign(&tmp[13]);
			continue;
		}
		else if(strncmp("InvertedFilePath",tmp,16)==0)
		{
			CInvertedFile invertedFile;
			invertedFile.m_fileName.assign(&tmp[17]);
			char tmpchr[50];
			strcpy(tmpchr,&tmp[17+invertedFile.m_fileName.length()+1]);
			invertedFile.m_headPage.m_pageSize=atoi(tmpchr);
			if(invertedFile.m_headPage.m_pageSize%512!=0)
				return BADCONFIGSETTING;
			this->insert(InvertedFile_Pair(fileNo.GetNum16(),invertedFile));
			m_largestFile=fileNo.GetNum16();
			++fileNo;
			continue;
		}
		else if(strncmp("MemeryLoad",tmp,10)==0)
		{
			this->m_memLoad=atoi(&tmp[11]);
			continue;
		}
	}
	fclose( stream );
	return OK;
}
const string& CConfigFile::GetWordFileName(void) const
{
	return this->m_wordFileName;
}
const Status CConfigFile::BuildInvertedFiles(void)
{
	CConfigFile::iterator iter=this->begin();
	Status status=OK;
	for(;iter!=this->end();iter++)
	{
		if((status=iter->second.CreateInvertedFile(iter->second.GetFileName(),iter->second.GetPageSize()))!=OK)
			return status;
	}
	return OK;
}
const Status CConfigFile::RemoveInvertedFiles(void)
{
	CConfigFile::iterator iter=this->begin();
	Status status=OK;
	for(;iter!=this->end();iter++)
	{
		if((status=iter->second.DeleteInvertedFile(iter->second.GetFileName()))!=OK)
			return status;
	}
	return OK;
}
const unsigned short CConfigFile::FindFitInvertedFile(const uint32_t urlsLen)
{
	unsigned short invertedFileID=0;
	CConfigFile::iterator iter=this->begin();
	for(;iter!=this->end();iter++)
	{
		if(iter->second.GetPageSize()>=urlsLen)
		{
			invertedFileID=iter->first;
			return invertedFileID;
		}
	}
	return this->m_largestFile;
}
const Status CConfigFile::OpenInvertedFiles(void)
{
	Status status=OK;
	CConfigFile::iterator iter=this->begin();
	for(;iter!=this->end();iter++)
	{
		if((status=iter->second.OpenInvertedFile())!=OK)
			return status;
	}
	return OK;
}
const Status CConfigFile::CloseInvertedFiles(void)
{
	Status status=OK;
	CConfigFile::iterator iter=this->begin();
	for(;iter!=this->end();iter++)
	{
		if((status=iter->second.CloseInvertedFile())!=OK)
			return status;
	}
	return OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Implementation of Class CFileOpTest./////////////////////////////////////
void CFileOpTest::RunTest() const
{
	cout<<"Size of CHeadPage:"<<HeadPageSize<<endl;
	cout<<"Size of CByteUInt:"<<sizeof(CByteUInt)<<endl;
	cout<<"Size of CWordInfo:"<<CWordInfoSize<<endl;
	cout<<"Actual size of CWordInfo:"<<sizeof(CWordInfo)<<endl;
	Error error;
	Status status=OK;
	CConfigFile configFile;
	CInvertedFile op1;
	string filename("invertedfile");
	op1.DeleteInvertedFile(filename);
	error.MyAssertOK(op1.CreateInvertedFile(filename,512 ));
	cout<<"Create Inverted File successfully."<<endl;
	CInvertedFile* op2=new CInvertedFile(filename,status);
	error.MyAssertOK(status);
	cout<<"Open Inverted File successfully."<<endl;
	assert(0==op2->m_headPage.m_nextFreePage);
	assert(1==op2->GetPageCnt());
	assert(512 ==op2->GetPageSize());
	uint64_t pageNo=0;
	char tmpchr[20];
	CIndexData index(20,tmpchr);
	error.MyAssertOK(op2->AllocateNewPages(pageNo,&index));
	assert(pageNo==1);
	cout<<"Read Inverted File's information successfully."<<endl;
	delete op2;
	/*error.MyAssertOK(op1.DeleteInvertedFile(filename));
	CPage page1(512);
	error.MyAssertOK(configFile.ReadConfigFile());
	cout<<"Read ConfigFile successfully."<<endl;
	cout<<"Word file's name:"<<configFile.GetWordFileName().c_str()<<endl;
	cout<<"Memery Load rate:"<<configFile.GetMemLoad()<<"%"<<endl;
	CConfigFile::iterator iter=configFile.begin();
	for(;iter!=configFile.end();iter++)
	{
		cout<<"File No:"<<iter->first<<", "<<"File name:"<<iter->second.GetFileName().c_str()<<", "<<"Page size:"<<iter->second.GetPageSize()<<endl;
	}
	configFile.RemoveInvertedFiles();
	error.MyAssertOK(configFile.BuildInvertedFiles());
	cout<<"Build Inverted Files successfully."<<endl;
	error.MyAssertOK(configFile.RemoveInvertedFiles());
	cout<<"Remove Inverted Files successfully."<<endl;
	CWordFile wordfile;
	error.MyAssertOK(wordfile.CreateWordFile(configFile.GetWordFileName()));
	cout<<"Build Word File successfully."<<endl;
	error.MyAssertOK(wordfile.DeleteWordFile(configFile.GetWordFileName()));
	cout<<"Remove Word File successfully."<<endl;
	error.MyAssertOK(configFile.BuildInvertedFiles());*/
	return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
