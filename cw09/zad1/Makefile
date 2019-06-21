GCC=gcc -Wall
SRC=main.c
OUT=main
LIB=-lpthread

build: $(SRC)
	$(GCC) $(SRC) -o $(OUT) $(LIB)

clean: $(OUT)
	rm -rf $(OUT)

test: $(OUT)
	./$(OUT) 10 3 3 4
