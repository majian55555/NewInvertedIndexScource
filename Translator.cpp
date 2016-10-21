#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Buffer.h"
#include "Translator.h"

CTranslator::CTranslator()
{
}
CTranslator::~CTranslator()
{
}
const bool CTranslator::ConnectMySQL()
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
	cout<<"Connect MySQL successfully!!!"<<endl;
	return true;
}
void CTranslator::CloseMySQL()
{
	mysql_free_result(this->m_res);
	mysql_close(&this->m_mysql);
}
unsigned int CTranslator::GetSites(unsigned int word_id, FILE *f, FILE *fs, unsigned int *&sites, unsigned int *counts) const
{
        //f is 00w/ind 
        // ci number and offset ,length
        // + 100 detal file
	unsigned int off = (word_id / 100) * sizeof(WordInd);
	unsigned int numSites = 0;
	if (off > 0)
		fseek(f, off - sizeof(WordInd), SEEK_SET);
	if ((off == 0) || (ftell(f) == (signed long)(off - sizeof(WordInd))))
	{
		WordInd ui[2];
		int elems;
		if (off > 0)
			elems = fread(ui, sizeof(WordInd), 1, f);
		else
		{
			ui[0].m_offset = 0;
			elems = 1;
		}
		if (elems && fread(ui + 1, sizeof(WordInd), 1, f)) //curse is later
		{
			counts[0] += ui[1].m_totalCount;
			counts[1] += ui[1].m_urlCount;
			numSites = (ui[1].m_offset - ui[0].m_offset) / (sizeof(unsigned int) << 1); // receive max
			if (numSites > 0)
			{
				sites = new unsigned int[(numSites << 1) + 1];
				if (ui[0].m_offset > 0)
					fseek(fs, ui[0].m_offset - sizeof(unsigned int), SEEK_SET);
				if (ui[0].m_offset == 0)
					sites[0] = 0;
				else
					fread(sites, sizeof(unsigned int), 1, fs);
                                //fs is sites of file 
				fread(sites + 1, sizeof(unsigned int) << 1, numSites, fs);
			}
		}
	}
	return numSites; // return mou yige wordid de site number
}
unsigned int CTranslator::GetSites(unsigned int word_id, unsigned int *&sites, unsigned short *&urls, unsigned int *counts,unsigned int& urlLen) const
{
	string DBWordDir("/usr/local/aspseek/var/aspseek");
	unsigned int siteLen = 0;
	unsigned int nfile = word_id % 100;
	char ind[50], sitesn[50], urlsn[50];
	sprintf(ind, "%s/%02uw/ind", DBWordDir.c_str(), nfile);
	sprintf(sitesn, "%s/%02uw/sites", DBWordDir.c_str(), nfile);
	sprintf(urlsn, "%s/%02uw/urls", DBWordDir.c_str(), nfile);
	FILE* f = fopen(ind, "r");
	FILE* fs = fopen(sitesn, "r");
	FILE* fu = fopen(urlsn, "r");
	if (f && fs && fu)
	{
		setvbuf(fu, NULL, _IONBF, 0);
		counts[0] = counts[1] = 0;
		unsigned int numSites = this->GetSites(word_id, f, fs, sites, counts);
		if (numSites)
		{
			siteLen = numSites * (sizeof(unsigned int) << 1);
                        //site length
			urlLen = sites[numSites << 1] - sites[0];
                        // url length
			urls = (unsigned short*)(new char[urlLen]);
			fseek(fu, sites[0], SEEK_SET);
			fread(urls, 1, urlLen, fu);
			for(unsigned int i=0;i<numSites;i++)
				sites[i<<1]+=siteLen;
		}
	}
	if (f) fclose(f);
	if (fs) fclose(fs);
	if (fu) fclose(fu);
	return siteLen;
}
CIndexData* CTranslator::TransData(unsigned int word_id,CWordInfo& wordinfo) const
{
	unsigned int counts[2];
	counts[0]=counts[1]=0;
	unsigned int* sites=NULL;
	unsigned short* urls=NULL;
	unsigned int urlLen =0;
	unsigned int siteLen=this->GetSites(word_id,sites,urls,counts,urlLen);
	if(siteLen&&urlLen)
	{
		sites[0]=siteLen;
		char* tmpresult=new char[siteLen+urlLen];
		memcpy(tmpresult,sites,siteLen);
		memcpy(&tmpresult[siteLen],urls,urlLen);
		CIndexData* result=new CIndexData(siteLen+urlLen,tmpresult);
		wordinfo.m_totalCount=counts[0];
		wordinfo.m_urlCount=counts[1];
		if(tmpresult) delete [] tmpresult;
		if(sites) delete [] sites;
		if(urls) delete [] urls;
		return result;
	}
	return NULL;
}
void CTranslator::Run()
{
	Error error;
	Status status=OK;
	CConfigFile configFile;
  error.MyAssertOK(configFile.ReadConfigFile());
  configFile.RemoveInvertedFiles();
	error.MyAssertOK(configFile.BuildInvertedFiles());
	cout<<"Build Inverted Files successfully."<<endl;
	CWordFile wordfile;
	wordfile.DeleteWordFile(configFile.GetWordFileName());
  error.MyAssertOK(wordfile.CreateWordFile(configFile.GetWordFileName()));
	cout<<"Build Word File successfully."<<endl;
	CBufferMgr* bufferMgr=new CBufferMgr(status);
	error.MyAssertOK(status);
	if(!this->ConnectMySQL())
		return;
	string sqlquery = "select word_id,word from wordurl;";
	if (mysql_query(&this->m_mysql, sqlquery.c_str()))
	{
		cerr<<mysql_error(&this->m_mysql)<<endl;
		return;
	}
	time_t timeBegin=time(NULL),timeFinish;
	this->m_res = mysql_use_result(&this->m_mysql);
	for(int i=0;(this->m_row = mysql_fetch_row(this->m_res)) != NULL;i++)	//get next line
	{
		unsigned long* len=mysql_fetch_lengths(this->m_res); //get each length
		unsigned int word_id = atoi(this->m_row[0]);
		string word(this->m_row[1],len[1]);
		//cout<<"Word_ID:"<<this->m_row[0]<<endl<<"Word:"<<word.c_str()<<endl;
		CWordInfo wordinfo;
		CIndexData* index=this->TransData(word_id,wordinfo);
		if(index)
		{
			CBufferID filepageID(0,0);
			error.MyAssertOK(bufferMgr->AllocPage(filepageID,index));
			error.MyAssertOK(bufferMgr->UnPinPage(filepageID,false));
			wordinfo.m_fileNo=filepageID.GetFileID();
			wordinfo.m_pageNo=filepageID.GetPageNo();
			error.MyAssertOK(wordfile.AddNewWord(word,wordinfo));
			//cout<<i<<"\tOK"<<endl;
		}
		else
		{
			cout<<i<<"\tNo"<<endl;
			continue;
		}
	}
	this->CloseMySQL();
	error.MyAssertOK(wordfile.FlushMenToFile(configFile.GetWordFileName()));
	cout<<"Flush Word File successfully."<<endl;
	error.MyAssertOK(bufferMgr->FlushAll());
	cout<<"Flush Inverted Files successfully."<<endl;
	delete bufferMgr;
	cout<<"Delete buffer manager successfully."<<endl;
	timeFinish=time(NULL);
	cout<<"Translating time:"<<timeFinish-timeBegin<<" seconds."<<endl;
}
