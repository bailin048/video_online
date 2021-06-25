LFLAG = -L /usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread
main:main.cpp db.hpp httplib.h
	g++ -std=c++11  $^ -o $@  $(LFLAG)
