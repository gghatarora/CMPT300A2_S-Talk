all: build

build:
	gcc -g -std=gnu99 -Wall -Werror udp_pThread.c udp.c IOThreads.c -o udp_pThread list/list.o -pthread 

clean:
	rm -f udp_pThread.o
