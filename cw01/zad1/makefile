CC = gcc
FILES = find_library.c
OUT_O = find_library.o
OUT_LIB = libfind_library.a
OUT_SHARED = libfind_library.so 

build: $(FILES)
	# create c files
	$(CC) -fPIC -c -o $(OUT_O) $(FILES)
	# create static library
	ar rcs $(OUT_LIB) $(OUT_O)
	# create shared library
	$(CC) -shared -o $(OUT_SHARED) $(OUT_O)

clean:
	rm -f *.o core

rebuild: clean build
