# DEBUG = -DDEBUG -g
DEBUG = 

all: base.o client
	g++ -o bin/base_server src/base_server.cpp build/base.o $(DEBUG)
	g++ -o bin/thread_server src/thread_server.cpp build/base.o -lpthread $(DEBUG)
	g++ -o bin/select_server src/select_server.cpp build/base.o $(DEBUG)
	g++ -o bin/poll_server src/poll_server.cpp build/base.o $(DEBUG)
	g++ -o bin/epoll_server src/epoll_server.cpp build/base.o -lpthread $(DEBUG)
	@echo DONE!

base.o:
	-mkdir build
	g++ -c src/base.cpp -o build/base.o $(DEBUG)

client: base.o
	-mkdir bin
	g++ -o bin/client src/client.cpp build/base.o $(DEBUG)
	cp -f src/clients.sh bin/clients.sh
	chmod 555 bin/clients.sh

.PHONY : clean
clean:
	-rm -rf build
	-rm -rf bin
