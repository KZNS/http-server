all: http-server

http-server: src/*.c src/*.h
	gcc -Wall -g src/*.c -o http-server -lssl -lcrypto -lpthread

clean:
	@rm http-server
