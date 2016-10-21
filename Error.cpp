#include "Error.h"
#include <errno.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

Error::Error(void)
{
}

Error::~Error(void)
{
}

void Error::printSysError(void) const
{
	switch(errno)
	{
	case E2BIG:cout<<"Argument list too long."<<endl;break;
	case EACCES:cout<<"Permission denied."<<endl;break;
	case EAGAIN:cout<<"No more processes or not enough memory or maximum nesting level reached."<<endl;break;
	case EBADF:cout<<"Bad file number."<<endl;break;
	case ECHILD:cout<<"No spawned processes."<<endl;break;
	case EDEADLOCK:cout<<"Resource deadlock would occur."<<endl;break;
	case EDOM:cout<<"Math argument."<<endl;break;
	case EEXIST:cout<<"File exists."<<endl;break;
	case EINVAL:cout<<"Invalid argument."<<endl;break;
	case EMFILE:cout<<"Too many open files."<<endl;break;
	case ENOENT:cout<<"No such file or directory."<<endl;break;
	case ENOEXEC:cout<<"Exec format error."<<endl;break;
	case ENOMEM:cout<<"Not enough memory."<<endl;break;
	case ENOSPC:cout<<"No space left on device."<<endl;break;
	case ERANGE:cout<<"Result too large."<<endl;break;
	case EXDEV:cout<<"Cross-device link."<<endl;break;
	default:cout<<"Unknown system error."<<endl;break;
	}
}

void Error::printError(Status status) const
{
	switch(status)
	{
	case SYSERR:
		this->printSysError();
		break;
	case BADFILENAME:
		cout<<"Bad File Name."<<endl;
		break;
	case FILENOTEXIST:
		cout<<"File does not exist."<<endl;
		break;
	case FILENOTOPEN:
		cout<<"File hasn't opened."<<endl;
		break;
	case FILEISOPEN:
		cout<<"File has opened."<<endl;
		break;
	case BADFILEID:
		cout<<"Bad file ID."<<endl;
		break;
	case BADCONFIGSETTING:
		cout<<"Bad config setting."<<endl;
		break;
	case WORDEXIST:
		cout<<"Word file exists."<<endl;
		break;
	case WORDNOTEXIST:
		cout<<"Word file does not exist."<<endl;
		break;
	case HASHNOTFOUND:
		cout<<"Hash not found."<<endl;
		break;
	case PAGENOTPINED:
		cout<<"Page hasn't pined."<<endl;
		break;
	case PAGEPINED:
		cout<<"Page has pined."<<endl;
		break;
	case ALLPAGESPINED:
		cout<<"All pages have pined."<<endl;
		break;
	case MEMNOTENOUGH:
		cout<<"Memory not enough."<<endl;
		break;
	case BUFFERNOTVALID:
		cout<<"Buffer not valid."<<endl;
		break;
	default:
		cout<<"Unknown error."<<endl;
		break;
	}
}

void Error::MyAssertOK(const Status& status) const
{
	if(status==OK)
		return;
	Error error;
	error.printError(status);
	exit( 1 );
}
