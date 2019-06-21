GCC = gcc
SRC = program.c
OUT = program
HIER_CREATOR = hierarchy.sh

build: $(SRC)
	$(GCC) -Wall $(SRC) -o $(OUT)

build_tree:
	@./$(HIER_CREATOR)

test: top 
	./$(OUT) top
