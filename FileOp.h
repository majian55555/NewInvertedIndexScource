
#ifndef FILEOP_H
#define FILEOP_H

#include "Pages.h"
#include "Defines.h"
#include <tr1/unordered_map>

using namespace std;
using namespace std::tr1;

const string ConfigFileName="InvertedFile.conf";

class CFileOp	// This class represents the file operation.
{
public:
	int m_fileHandle;	// File handle from Operating System.
public:
	CFileOp(void);	// Construction fuction.
	~CFileOp(void);	// Distruction fuction.
	const Status CreateFile(const string& fileName);	// Create a new file.
	const Status DeleteFile(const string& fileName);	// Delete this file.
	const Status OpenFile(const string& fileName);	// Open this file with O_DIRECT.
	const Status OpenFile2(const string& fileName);	// Open this file without O_DIRECT.
	const Status CloseFile(void);	// Close this file.
	const Status ReadFile(void* buffer,const int64_t& offset,const int32_t length) const;	// Read file's data to buffer.
	const Status WriteFile(const void* buffer,const int64_t& offset,const int32_t length) const;	// Write buffer to dile.
	const Status ExtendFile(const int32_t size,const void* buffer=NULL) const;	// Append size in this file.
	const Status TruncFile(const int32_t size,const string& fileName);	// Truncate this file into size.
	const Status GetFileSize(int64_t& fileSize) const;	// Get this file's size.
	const bool IsOpen() const
	{
		if(this->m_fileHandle<=0)
			return false;
		return true;
	}
};

class CInvertedIndex	// This class represents the inverted index of each words.
{
private:
	uint32_t m_length;	// The length of this inverted index.
	char* m_data;	// Actual data.
public:
	CInvertedIndex(const char* data);
	CInvertedIndex(const uint32_t len,const char* data);
	const uint32_t GetLength() const;
	const char* GetData() const;
	void ShowUrls() const;
};

class CInvertedFile	// This class represents the inverted index of each words.
{
	friend class CConfigFile;
	friend class CFileOpTest;
private:
	string m_fileName;	// File path and file name.
	CHeadPage m_headPage;
	CFileOp m_fileOp;
	CInvertedFile();
public:
	CInvertedFile(const string& fileName,Status& status);	// Construction fuction.
	~CInvertedFile();	// Distruction fuction.
	const Status OpenInvertedFile(void);
	const Status CloseInvertedFile(void);
	const string GetFileName(void) const;
	const uint32_t GetPageSize(void) const;
	const uint64_t GetPageCnt(void) const;
	const Status CreateInvertedFile(const string& fileName,const uint32_t pageSize);
	const Status DeleteInvertedFile(const string& fileName);
	const Status AllocateNewPages(uint64_t& pageNo,const CIndexData* index);	// Allocate a new page.
	const Status DisposePages(const uint64_t pageNo);	// Dispose a page.
	const Status ReadIndex(const uint64_t pageNo,CIndexData* index) const;	// Read file's page to buffer.
	const Status WriteIndex(const uint64_t pageNo,const CIndexData* index);	// Write buffer to file.
	const Status FlushHeadInfo(void);
};

class CWordFile : public unordered_map<string,CWordInfo>	// This class represents the dictionary file.
{ // The hash map of every word and it's pointer to inverted file(contains file ID and page number).
private:
	CFileOp m_fileOp;
	bool m_modified;
public:
	CWordFile();
	~CWordFile();
	const Status CreateWordFile(const string& wordfilename);
	const Status DeleteWordFile(const string& wordfilename);
	const Status AddNewWord(const string& word,const CWordInfo& wordinfo);
	const Status UpdateWord(const string& word,const CWordInfo& wordinfo);
	const Status DeleteWord(const string& word);
	const Status ReadFileToMem(const string& wordfilename);
	const Status FlushMenToFile(const string& wordfilename);
};

typedef pair <string, CWordInfo> WordInfo_Pair;

class CConfigFile : public unordered_map<unsigned short,CInvertedFile>	// This class represents the hash map of inverted file ID and inverted file path.
{
private:
	string m_wordFileName;	// The dictionary file's path and name.
	uint32_t m_memLoad;
	unsigned short m_largestFile;
public:
	CConfigFile(void);
	~CConfigFile(void);
	const uint32_t GetMemLoad() const
	{
		return this->m_memLoad;
	}
	const Status ReadConfigFile(void);
	const string& GetWordFileName(void) const;
	const Status BuildInvertedFiles(void);
	const Status RemoveInvertedFiles(void);
	const unsigned short FindFitInvertedFile(const uint32_t urlsLen);
	const Status OpenInvertedFiles(void);
	const Status CloseInvertedFiles(void);
};

typedef unordered_map<unsigned short,CInvertedFile>::iterator InvertedFile_Iter;
typedef pair <unsigned short, CInvertedFile> InvertedFile_Pair;

class CFileOpTest
{
public:
	void RunTest(void) const;
};

#endif
