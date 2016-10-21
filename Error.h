#ifndef ERROR_H
#define ERROR_H

enum Status
{
	OK = 0,
	//Error status of IO layer
	SYSERR = -999, BADFILENAME = -998, FILENOTEXIST = -997, FILENOTOPEN = -991,FILEISOPEN = -984, BADFILEID = -974, 
	BADCONFIGSETTING = -973, WORDEXIST = -972, WORDNOTEXIST = -971, HASHNOTFOUND = -970, PAGENOTPINED = -969, 
	PAGEPINED = -968, MEMNOTENOUGH = -967, ALLPAGESPINED = -966, BUFFERNOTVALID = -965
};
class Error	// This class handles all errors.
{
public:
	Error(void);
	~Error(void);
	void printError(Status status) const;
	void printSysError() const;
	void MyAssertOK(const Status& status) const;
};

#endif
