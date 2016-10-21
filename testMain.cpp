#include "Buffer.h"
#include "Translator.h"
#include "Compare.h"

int main(int argc,char **argv)
{
	/*CDefinesTest definesTest;
	CPagesTest pagesTest;
	CFileOpTest fileOpTest;
	CBufferTest bufferTest;
	definesTest.RunTest();
	pagesTest.RunTest();
	fileOpTest.RunTest();
	bufferTest.RunTest();*/
	//CTranslator translator;
	//translator.Run();
	CCompare compTest;
	vect_str testVect;
	compTest.CreateTestVector4(testVect);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,10000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,100000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,200000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,500000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,1000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,2000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,3000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,4000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,5000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,6000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,7000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,8000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,9000000);
	//system("/etc/init.d/mysqld restart");
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	compTest.RunNew(testVect,10000000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,1000000);
	//compTest.RunOld(testVect,50000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,50000);
	//compTest.RunOld(testVect,100000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,100000);
	//compTest.RunOld(testVect,150000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,150000);
	//compTest.RunOld(testVect,200000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,200000);	
	//compTest.RunOld(testVect,300000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,300000);
	//compTest.RunOld(testVect,400000);
	//system("sync");
	//system("echo 3 > /proc/sys/vm/drop_caches");
	//system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,400000);
	//compTest.RunOld(testVect,500000);
	/*system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,500000);
	compTest.RunOld(testVect,700000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,700000);
	compTest.RunOld(testVect,1000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,1000000);
	compTest.RunOld(testVect,2000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,2000000);
	compTest.RunOld(testVect,3000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,3000000);
	compTest.RunOld(testVect,4000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,4000000);
	compTest.RunOld(testVect,5000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,5000000);
	compTest.RunOld(testVect,6000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,6000000);
	compTest.RunOld(testVect,7000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,7000000);
	compTest.RunOld(testVect,8000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,8000000);
	compTest.RunOld(testVect,9000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,9000000);
	compTest.RunOld(testVect,10000000);
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	system("/etc/init.d/mysqld restart");
	//compTest.RunNew(testVect,10000000);*/
	return 0;
}

	
	
