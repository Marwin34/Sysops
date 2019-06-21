GCC=gcc -Wall
SERVER_SRC=server.c
CLIENT_SRC=client.c
SERVER_OUT=server
CLIENT_OUT=client
LIB=-lpthread

build: $(SERVER_SRC)
	$(GCC) $(SERVER_SRC) -o $(SERVER_OUT) $(LIB)
	$(GCC) $(CLIENT_SRC) -o $(CLIENT_OUT) $(LIB)

run_server: $(SERVER_OUT)
	./$(SERVER_OUT) 8888 testserver

clean: $(SERVER_OUT)
	rm -rf $(SERVER_OUT)
