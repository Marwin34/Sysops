SERVER_SRC=server.c
CLIENT_SRC=client.c
SERVER_OUT=server
CLIENT_OUT=client
GCC=gcc -Wall
LIB=-lrt

build: $(SERVER_SRC) $(CLIENT_SRC)
	$(GCC) $(SERVER_SRC) -o $(SERVER_OUT) $(LIB)
	$(GCC) $(CLIENT_SRC) -o $(CLIENT_OUT) $(LIB)

run_client:
	./$(CLIENT_OUT)

run_server:
	./$(SERVER_OUT)

clean:
	rm -rf $(CLIENT_OUT) $(SERVER_OUT)