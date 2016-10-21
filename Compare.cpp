#include <stdlib.h>
#include <iconv.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Buffer.h"
#include "Compare.h"
#include "Translator.h"

///////////////////////////////////Implementation of Class CFileOp//////////////////////////////////////
const bool CCompare::ConnectMySQL()
{
	const char* host = "localhost";
	const char* user = "aspseek";
	const char* password = "BCICd10A631x5h6";
	const char* dbname = "aspseek";
	int port = 0;  
	mysql_init(&this->m_mysql);
	if(NULL==mysql_real_connect(&this->m_mysql, host, user, password, dbname, port, NULL, 0))
	{
		cerr<<"Could not connect MySQL!!!"<<endl;
		return false; 
	}
	//cout<<"Connect MySQL successfully!!!"<<endl;
	return true;
}
void CCompare::CloseMySQL()
{
	if(this->m_res)
		mysql_free_result(this->m_res);
	mysql_close(&this->m_mysql);
}
const int CCompare::RandomNum(const int rangemin,const int rangemax)
{
	if(this->m_firstRan)
	{
		srand((unsigned)time(NULL));
		this->m_firstRan=false;
	}
	int result=(int)(((double) rand() / (double) RAND_MAX) * rangemax + rangemin);
	return result;
}
const bool CCompare::CreateTmpWord(const int num)
{
	Error error;
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem("/usr/local/NewInvertedIndex/var/wordFile.data"));
	cout<<"Read Words File successfully."<<endl;
	int maxid=wordFile.size()-1,minid=0;
	if(!this->ConnectMySQL())
		return false;
	string sqlquery="drop table tmpword;";
	mysql_real_query(&this->m_mysql, sqlquery.c_str(),sqlquery.length());
	sqlquery="create table tmpword(word_id int(11) not null,word varchar(50) not null);";
	if (mysql_real_query(&this->m_mysql, sqlquery.c_str(),sqlquery.length()))
	{
		cerr<<mysql_error(&this->m_mysql)<<endl;
		return false;
	}
	unordered_map<string,CWordInfo>::iterator iter=wordFile.begin();
	for(int i=0;i<num;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		sqlquery="insert into tmpword values(";
		char tmpchr[10];
		sprintf(tmpchr,"%d",i);
		sqlquery.append(tmpchr);
		sqlquery.append(",\'");
		iter=wordFile.begin();
		for(int j=0;j<randid;j++)
			++iter;
		//iter+=randid;
		sqlquery.append(iter->first);
		sqlquery.append("\');");
		//cout<<sqlquery.c_str()<<endl;
		if (mysql_real_query(&this->m_mysql, sqlquery.c_str(),sqlquery.length()))
		{
			cerr<<mysql_error(&this->m_mysql)<<endl;
			return false;
		}
		cout<<i<<endl;
	}
	mysql_close(&this->m_mysql);
	return true;
}
unsigned int CCompare::GetSites(unsigned int word_id, int fi, int fs, unsigned int *&sites, unsigned int *counts) const
{
	unsigned int off = (word_id / 100) * sizeof(WordInd);
	unsigned int numSites = 0;
	int seekstatus=0;
	if (off > 0)
		seekstatus=::lseek(fi, off - sizeof(WordInd), SEEK_SET);
	if ((off == 0) || (seekstatus!=-1))
	{
		WordInd ui[2];
		int elems;
		if (off > 0)
			elems = ::read(fi, ui, sizeof(WordInd));
		else
		{
			ui[0].m_offset = 0;
			elems = 1;
		}
		if (elems && ::read(fi, ui + 1, sizeof(WordInd))==sizeof(WordInd))
		{
			counts[0] += ui[1].m_totalCount;
			counts[1] += ui[1].m_urlCount;
			numSites = (ui[1].m_offset - ui[0].m_offset) / (sizeof(unsigned int) << 1);
			if (numSites > 0)
			{
				sites = new unsigned int[(numSites << 1) + 1];
				if (ui[0].m_offset > 0)
					::lseek(fs, ui[0].m_offset - sizeof(unsigned int), SEEK_SET);
				if (ui[0].m_offset == 0)
					sites[0] = 0;
				else
					::read(fs, sites, sizeof(unsigned int));
				::read(fs, sites + 1, (sizeof(unsigned int) << 1)*numSites);
			}
		}
	}
	//posix_fadvise(fi, 0, 0, POSIX_FADV_DONTNEED);
	//posix_fadvise(fs, 0, 0, POSIX_FADV_DONTNEED);
	return numSites;
}
unsigned int CCompare::GetSites(unsigned int word_id, unsigned int *&sites, unsigned short *&urls, unsigned int *counts,unsigned int& urlLen) const
{
	string DBWordDir("/usr/local/aspseek/var/aspseek");
	unsigned int siteLen = 0;
	unsigned int nfile = word_id % 100;
	char ind[50], sitesn[50], urlsn[50];
	sprintf(ind, "%s/%02uw/ind", DBWordDir.c_str(), nfile);
	sprintf(sitesn, "%s/%02uw/sites", DBWordDir.c_str(), nfile);
	sprintf(urlsn, "%s/%02uw/urls", DBWordDir.c_str(), nfile);
	int fi = ::open(ind,O_RDWR);
	int fs = ::open(sitesn,O_RDWR);
	int fu = ::open(urlsn,O_RDWR);
	if (-1!=fi && -1!=fs && -1!=fu)
	{
		counts[0] = counts[1] = 0;
		unsigned int numSites = this->GetSites(word_id, fi, fs, sites, counts);
		if (numSites)
		{
			siteLen = numSites * (sizeof(unsigned int) << 1);
			urlLen = sites[numSites << 1] - sites[0];
			urls = (unsigned short*)(new char[urlLen]);
			::lseek(fu, sites[0], SEEK_SET);
			::read(fu, urls, urlLen);
			//posix_fadvise(fu, 0, 0, POSIX_FADV_DONTNEED);
		}
	}
	if (fi!=-1) ::close(fi);
	if (fs!=-1) ::close(fs);
	if (fu!=-1) ::close(fu);
	return siteLen;
}
void CCompare::RunOld(const vect_str& testvect, const unsigned int looptime)
{
	cout<<"Testing old inverted index, loop times:"<<looptime<<endl;
	if(!this->ConnectMySQL())
		return;
	string sqlquery;
	int maxid=testvect.size()-1,minid=0;
	time_t timeBegin=time(NULL),timeFinish;
	vect_str::const_iterator iter=testvect.begin();
	for(unsigned int i=0;i<looptime;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		sqlquery="select word_id from wordurl where word=\'";
		sqlquery.append(*(iter+randid));
		sqlquery.append("\';");
		//cout<<sqlquery.c_str()<<endl;
		if (mysql_query(&this->m_mysql, sqlquery.c_str()))
		{
			cerr<<mysql_error(&this->m_mysql)<<endl;
			return;
		}
		this->m_res = mysql_use_result(&this->m_mysql);
		if((this->m_row = mysql_fetch_row(this->m_res)) == NULL)
		{
			cerr<<"Could not find this word."<<endl;
			return;
		}
		unsigned int word_id=atoi(this->m_row[0]);
		mysql_free_result(this->m_res);
		unsigned int counts[2];
		counts[0]=counts[1]=0;
		unsigned int* sites=NULL;
		unsigned short* urls=NULL;
		unsigned int urlLen =0;
		unsigned int siteLen=this->GetSites(word_id, sites, urls, counts, urlLen);
		if(siteLen)
		{
			delete [] sites;
			delete [] urls;
		}
	}
	mysql_close(&this->m_mysql);
	timeFinish=time(NULL);
	cout<<"Old inverted file use:"<<timeFinish-timeBegin<<" seconds."<<endl;
}
void CCompare::RunNew(const vect_str& testvect, const unsigned int looptime)
{
	cout<<"Testing new inverted index, loop times:"<<looptime<<endl;
	Error error;
	Status status=OK;
	CBufferMgr* bufferMgr=new CBufferMgr(status);
	error.MyAssertOK(status);
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem(bufferMgr->GetWordFileName()));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	int maxid=testvect.size()-1,minid=0;
	time_t timeBegin=time(NULL),timeFinish;
	vect_str::const_iterator iter=testvect.begin();
	for(unsigned int i=0;i<looptime;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		string queryword(*(iter+randid));
		unordered_map<string,CWordInfo>::const_iterator iter=wordFile.find(queryword);
		if(iter==wordFile.end())
		{
			cerr<<"Could not find this word."<<endl;
			return;
		}
		//cout<<(iter->second).m_fileNo.GetNum16()<<endl<<(iter->second).m_pageNo<<endl;
		CBufferID bufferID((iter->second).m_fileNo,(iter->second).m_pageNo);
		CIndexData* indexData=NULL;
                
		error.MyAssertOK(bufferMgr->ReadPage(bufferID,indexData));
		error.MyAssertOK(bufferMgr->UnPinPage(bufferID,false));
	}
	timeFinish=time(NULL);
	cout<<"New inverted file use:"<<timeFinish-timeBegin<<" seconds."<<endl;
	error.MyAssertOK(bufferMgr->FlushAll());
	cout<<"Flush Inverted Files successfully."<<endl;
	cout<<"Size of hash map:"<<bufferMgr->size()<<endl;
	delete bufferMgr;
	cout<<"Delete buffer manager successfully."<<endl;
}

void CCompare::RunNewNoCache(const vect_str& testvect, const unsigned int looptime)
{
	cout<<"Testing new inverted index without cache, loop times:"<<looptime<<endl;
	CWordFile wordFile;
	Error error;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem("/usr/local/NewInvertedIndex/var/wordFile.data"));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	CConfigFile configFile;
	error.MyAssertOK(configFile.ReadConfigFile());
	error.MyAssertOK(configFile.OpenInvertedFiles());
	int maxid=testvect.size()-1,minid=0;
	time_t timeBegin=time(NULL),timeFinish;
	vect_str::const_iterator iter=testvect.begin();
	for(unsigned int i=0;i<looptime;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		string queryword(*(iter+randid));
		unordered_map<string,CWordInfo>::const_iterator iter=wordFile.find(queryword);
		if(iter==wordFile.end())
		{
			cerr<<"Could not find this word."<<endl;
			return;
		}
		CBufferID bufferID((iter->second).m_fileNo,(iter->second).m_pageNo);
		unsigned short fileID=(iter->second).m_fileNo.GetNum16();
		uint64_t pageNo=(iter->second).m_pageNo;
		InvertedFile_Iter invertedFileIter=configFile.find(fileID);
		if(invertedFileIter==configFile.end())
		{
			cerr<<"Could not find this inverted file."<<endl;
			return;
		}
		CIndexData *indexData=new CIndexData;
		error.MyAssertOK(invertedFileIter->second.ReadIndex(pageNo,indexData));
		delete indexData;
	}
	error.MyAssertOK(configFile.CloseInvertedFiles());
	timeFinish=time(NULL);
	cout<<"New inverted file without cache use:"<<timeFinish-timeBegin<<" seconds."<<endl;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	char **pin = &inbuf;
	char **pout = &outbuf;
	iconv_t cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen)!=0)
	{
		iconv_close(cd);
		return -1;
	}
	iconv_close(cd);
	return 0;
}

const bool CCompare::CreateWordVector(vect_str& wordvect)
{
	Error error;
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem("/usr/local/NewInvertedIndex/var/wordFile.data"));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	cout<<"Reading Keyword File ....."<<endl;
	vector<string> vect_keyword;
	FILE *stream=NULL;
	char line[MAXNAMESIZE],out[MAXNAMESIZE];
	stream=fopen( "/usr/local/aspseek/etc/tables/keyword.txt", "r" );
	if( NULL==stream )
	{
		cout<<"Can't open keywords file:";
		return false;
	}
	string tmpstr;
	while(fgets( line, MAXNAMESIZE, stream ))
	{
		if (!line[0])
			continue;
		line[strlen(line)-1]='\0';
		if(code_convert("gb18030","utf-8",line,strlen(line),out,MAXNAMESIZE)==-1)
			continue;
		tmpstr.assign(out,strlen(out)-1);
		//cout<<out<<endl;
		vect_keyword.push_back(tmpstr);
	}
	
	fclose( stream );
	cout<<"Read KeyWord File successfully."<<endl;
	cout<<"Keyword vector size:"<<vect_keyword.size()<<endl;
	cout<<"Creating word vector ....."<<endl;
	vector<string>::iterator iter=vect_keyword.begin();
	for(;iter!=vect_keyword.end();++iter)
	{
		if(wordFile.find(*iter)!=wordFile.end())
			wordvect.push_back(*iter);
	}
	cout<<"Create word vector successfully."<<endl;
	cout<<"Word vector size:"<<wordvect.size()<<endl;
	return true;
}
void CCompare::CreateTestVector(const vect_str& wordvect, vect_str& testvect, const unsigned int looptime)
{
	int maxid=wordvect.size()-1,minid=0;
	cout<<"Creating test vector ....."<<endl;
	//vect_str::iterator iter=wordvect.begin();
	for(unsigned int i=0;i<looptime;++i)
	{
		int randid=this->RandomNum(minid,maxid);
		string tmpstr=wordvect.at(randid);
		testvect.push_back(tmpstr);
	}
	cout<<"Create test vector successfully."<<endl;
	cout<<"Test vector size:"<<testvect.size()<<endl;
}
void CCompare::CreateTestVector1(vect_str& testvect)
{
	Error error;
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem("/usr/local/NewInvertedIndex/var/wordFile.data"));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	vect_str tmpVect;
	unordered_map<string,CWordInfo>::iterator iter=wordFile.begin();
	for(;iter!=wordFile.end();iter++)
	{
		tmpVect.push_back(iter->first);
	}
	int maxid=tmpVect.size()-1,minid=0;
	vect_str::iterator tmpiter=tmpVect.begin();
	for(int i=0;i<50000;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		//cout<<(tmpiter+randid)->c_str()<<endl;
		testvect.push_back(*(tmpiter+randid));
	}
}
void CCompare::CreateTestVector2(vect_str& testvect)
{
	if(this->CreateWordVector(testvect))
		cout<<"CreateTestVector2 Successfully."<<endl;
	else
		cout<<"CreateTestVector2 Failed."<<endl;
}
void CCompare::CreateTestVector3(vect_str& testvect)
{
	vect_str tmpVect;
	if(this->CreateWordVector(tmpVect))
		cout<<"Create Word Vector Successfully."<<endl;
	else
		cout<<"Create Word Vector Failed."<<endl;
	int maxid=tmpVect.size()-1,minid=0;
	vect_str::iterator tmpiter=tmpVect.begin();
	for(int i=0;i<50000;i++)
	{
		int randid=this->RandomNum(minid,maxid);
		//cout<<(tmpiter+randid)->c_str()<<endl;
		testvect.push_back(*(tmpiter+randid));
	}
}
void CCompare::CreateTestVector4(vect_str& testvect)
{
	Error error;
	CWordFile wordFile;
	cout<<"Reading Words File ....."<<endl;
	error.MyAssertOK(wordFile.ReadFileToMem("/usr/local/NewInvertedIndex/var/wordFile.data"));
	cout<<"Read Words File successfully."<<endl;
	cout<<"Words' hash map size:"<<wordFile.size()<<endl;
	unordered_map<string,CWordInfo>::iterator iter=wordFile.begin();
	for(;iter!=wordFile.end();iter++)
	{
		testvect.push_back(iter->first);
	}
}
