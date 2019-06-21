GCC=gcc -Wall
SRC=program.c
SRC_b=program_b.c
OUT=program
OUT_b=program_b
SCRIPT=date.sh


build: $(SRC) $(SRC_b)
	$(GCC) $(SRC) -o $(OUT) 
	$(GCC) $(SRC_b) -o $(OUT_b)

clean:
	rm -rf *.o

test: $(OUT)
	./$(OUT)
	
test_b: $(OUT_b) $(SCRIPT)
	./$(OUT_b)