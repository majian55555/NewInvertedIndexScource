#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <mysql.h>
#include <mysqld_error.h>

class WordInd
{
public:
	unsigned int m_offset;
	unsigned int m_siteCount;
	unsigned int m_urlCount;
	unsigned int m_totalCount;
	WordInd()
	{
		m_offset = 0;
		m_siteCount = 0;
		m_urlCount = 0;
		m_totalCount = 0;
	}
};

class CSiteInfo
{
public:
	unsigned int m_site;			///< Site ID
	unsigned int m_offset;			///< Offset of URLs information in search results buffer
	unsigned int m_length;			///< Length of URLs information in search results buffer
};

class CTranslator
{
private:
	MYSQL m_mysql;
	MYSQL_RES *m_res;      
	MYSQL_ROW m_row;
	const bool ConnectMySQL();
	void CloseMySQL(void);
	unsigned int GetSites(unsigned int word_id, FILE* f, FILE* fs, unsigned int*& sites, unsigned int* counts) const;
	unsigned int GetSites(unsigned int word_id, unsigned int*& sites, unsigned short*& urls, unsigned int* counts,unsigned int& urlLen) const;
	void showSites(unsigned int siteNum,unsigned int* sites) const;
	CIndexData* TransData(unsigned int word_id,CWordInfo& wordinfo) const;
public:
	CTranslator(void);
	~CTranslator(void);
	void Run(void);
};

#endif
