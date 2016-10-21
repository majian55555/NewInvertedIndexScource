#ifndef COMPARE_H
#define COMPARE_H
#include <mysql.h>
#include <mysqld_error.h>
#include <vector>
using namespace std;
typedef vector<string> vect_str;
class CCompare
{
private:
	MYSQL m_mysql;
	MYSQL_RES *m_res;      
	MYSQL_ROW m_row;
	bool m_firstRan;
	const bool ConnectMySQL();
	void CloseMySQL(void);
	unsigned int GetSites(unsigned int word_id, int fi, int fs, unsigned int *&sites, unsigned int *counts) const;
	unsigned int GetSites(unsigned int word_id, unsigned int *&sites, unsigned short *&urls, unsigned int *counts,unsigned int& urlLen) const;
public:
	CCompare():m_res(NULL),m_firstRan(true){}
	const int RandomNum(const int rangemin,const int rangemax);
	const bool CreateTmpWord(const int num);
	const bool CreateWordVector(vect_str& wordvect);
	void CreateTestVector(const vect_str& wordvect, vect_str& testvect, const unsigned int looptime);
	void CreateTestVector1(vect_str& testvect);
	void CreateTestVector2(vect_str& testvect);
	void CreateTestVector3(vect_str& testvect);
	void CreateTestVector4(vect_str& testvect);
	void RunOld(const vect_str& testvect, const unsigned int looptime);
	void RunNew(const vect_str& testvect, const unsigned int looptime);
	void RunNewNoCache(const vect_str& testvect, const unsigned int looptime);
};
#endif
