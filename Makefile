DEBUG = -DDEBUG
# DEBUG = 

all: base.o client
	g++ -o bin/base_server src/base_server.cpp build/base.o $(DEBUG)
	g++ -o bin/thread_server src/thread_server.cpp build/base.o -lpthread $(DEBUG)
	g++ -o bin/select_server src/select_server.cpp build/base.o $(DEBUG)
	g++ -o bin/poll_server src/poll_server.cpp build/base.o $(DEBUG)
	g++ -o bin/epoll_server src/epoll_server.cpp build/base.o -lpthread $(DEBUG)
	cp -f src/clients.sh bin/clients.sh
	chmod +x bin/clients.sh
	@echo DONE!

base.o:
	-mkdir build
	g++ -c src/base.cpp -o build/base.o

client: base.o
	-mkdir bin
	g++ -o bin/client src/client.cpp build/base.o $(DEBUG)

.PHONY : clean
clean:
	-rm -r build
	-rm -r bin
