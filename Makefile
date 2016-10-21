testMain : testMain.o Defines.o Error.o Pages.o FileOp.o Buffer.o Translator.o Compare.o
	g++ -g -O0 -Wall -m64 -lstdc++ --std=gnu++0x -L/usr/lib64/mysql -lmysqlclient -I/usr/include/mysql Error.o Defines.o Pages.o FileOp.o Buffer.o Translator.o Compare.o testMain.o -o testMain
testMain.o : testMain.cpp
	g++ -g -O0 -Wall -m64 -I/usr/include/mysql -c testMain.cpp
Error.o : Error.cpp
	g++ -g -O0 -Wall -m64 -c Error.cpp
Defines.o : Defines.cpp
	g++ -g -O0 -Wall -m64 -c Defines.cpp
Pages.o : Pages.cpp
	g++ -g -O0 -Wall -m64 -c Pages.cpp
FileOp.o : FileOp.cpp
	g++ -g -O0 -Wall -m64 -c FileOp.cpp
Buffer.o : Buffer.cpp
	g++ -g -O0 -Wall -m64 -c Buffer.cpp
Translator.o : Translator.cpp
	g++ -g -O0 -Wall -m64 -I/usr/include/mysql -c Translator.cpp
Compare.o : Compare.cpp
	g++ -g -O0 -Wall -m64 -I/usr/include/mysql -c Compare.cpp
clean:
	-rm -f *.o
	-rm -f testMain
