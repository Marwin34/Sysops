CC = gcc
SRC = main.c
LIB = find_library
LIB_A = libfind_library.a
OUT_STATIC = main_static
OUT_SHARED = main_shared
SRC_DYNAMIC = main_dynamic.c
OUT_DYNAMIC = main_dynamic
PREFIX = LD_LIBRARY_PATH=../zad1

build: $(SRC)
	# compile with static library
	$(CC) -Wall -I../zad1 -o $(OUT_STATIC) $(SRC) ../zad1/$(LIB_A)

	# compile with shared library
	$(CC) -Wall -I../zad1 -L../zad1  -lfind_library -o $(OUT_SHARED) $(SRC) 

	#compile with dynamic library
	$(CC) -Wall -o $(OUT_DYNAMIC) $(SRC_DYNAMIC) -ldl
	
clean:
	rm -f *.o core

BLOCK_COUNT = 10
REPEAT_COUNT = 10

LOAD_COMMAND = $(shell for VARIABLE in {1..$(BLOCK_COUNT)}; do echo "load_to_memory";done)
REMOVE_COMMAND = $(shell for VARIABLE in {1..$(BLOCK_COUNT)}; do echo "remove_block `expr $$VARIABLE - 1`";done)
COMMAND = $(shell for VARIABLE in {1..$(REPEAT_COUNT)}; do echo $(LOAD_COMMAND) $(REMOVE_COMMAND);done )
test:
	# static tests
	# przeprowadzenie przeszukania katalogów o różnych poziomach zagłębień i różnych liczbach zawartych plików
	@ printf "test z biblioteka statyczna \n\ntest przeszukiwania katalogow" >> log
	@ echo "bardzo zagniezdzony plik" >> log
	@./main_static start_clock search_directory / libgcc_s.so result2 stop_clock log
	@echo "srednio zagniezdzony plik" >> log
	@./main_static start_clock search_directory /usr libgcc_s.so result2 stop_clock log 
	@ echo "nie zagniezdzony plik" >> log
	@./main_static start_clock search_directory /usr/lib libgcc_s.so result2 stop_clock log

	# zapisanie w pamięci bloków o różnych rozmiarach
	@ echo "test zapisu w pamieci" >> log
	@ echo "duzy plik" >> log
	@./main_static create_table 3 search_directory / libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "sredni plik" >> log
	@./main_static create_table 3 search_directory /usr libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "maly plik" >> log
	@./main_static create_table 3 search_directory /usr/lib libgcc_s.so result2 start_clock load_to_memory stop_clock log

	# usunięcie zaalokowanych bloków o różnych rozmiarach 
	@ echo "test usuwania blokow z pamieci" >> log
	@ echo "duzy plik" >> log
	@./main_static create_table 3 search_directory / libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "sredni plik" >> log
	@./main_static create_table 3 search_directory /usr libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "maly plik" >> log
	@./main_static create_table 3 search_directory /usr/lib libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log

	# na przemian  kilkakrotne dodanie i usunięcie zadanej liczby bloków 
	@ echo "test dodawania i usuwania zadanej liczby blokow" >> log
	@./main_static create_table $(BLOCK_COUNT) search_directory / libgcc_s.so result2 \
	start_clock \
	$(COMMAND) \
	stop_clock \
	log

	# shared tests
	# przeprowadzenie przeszukania katalogów o różnych poziomach zagłębień i różnych liczbach zawartych plików
	@ printf "\n\ntest z biblioteka wspoldzielona \n\ntest przeszukiwania katalogow" >> log
	@ echo "bardzo zagniezdzony plik" >> log
	@ $(PREFIX) ./main_shared start_clock search_directory / libgcc_s.so result2 stop_clock log
	@echo "srednio zagniezdzony plik" >> log
	@ $(PREFIX) ./main_shared start_clock search_directory /usr libgcc_s.so result2 stop_clock log 
	@ echo "nie zagniezdzony plik" >> log
	@ $(PREFIX) ./main_shared start_clock search_directory /usr/lib libgcc_s.so result2 stop_clock log

	# zapisanie w pamięci bloków o różnych rozmiarach
	@ echo "test zapisu w pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory / libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory /usr libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory /usr/lib libgcc_s.so result2 start_clock load_to_memory stop_clock log

	# usunięcie zaalokowanych bloków o różnych rozmiarach 
	@ echo "test usuwania blokow z pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory / libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory /usr libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./main_shared create_table 3 search_directory /usr/lib libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log

	# na przemian  kilkakrotne dodanie i usunięcie zadanej liczby bloków 
	@ echo "test dodawania i usuwania zadanej liczby blokow" >> log
	@ $(PREFIX) ./main_shared create_table $(BLOCK_COUNT) search_directory / libgcc_s.so result2 \
	start_clock \
	$(COMMAND) \
	stop_clock \
	log

	# dynamic test
	# przeprowadzenie przeszukania katalogów o różnych poziomach zagłębień i różnych liczbach zawartych plików
	@ printf "\n\ntest z biblioteka dynamiczna \n\ntest przeszukiwania katalogow" >> log
	@ echo "bardzo zagniezdzony plik" >> log
	@ $(PREFIX) ./main_dynamic start_clock search_directory / libgcc_s.so result2 stop_clock log
	@ echo "srednio zagniezdzony plik" >> log
	@ $(PREFIX) ./main_dynamic start_clock search_directory /usr libgcc_s.so result2 stop_clock log 
	@ echo "nie zagniezdzony plik" >> log
	@ $(PREFIX) ./main_dynamic start_clock search_directory /usr/lib libgcc_s.so result2 stop_clock log

	# zapisanie w pamięci bloków o różnych rozmiarach
	@ echo "test zapisu w pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory / libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory /usr libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory /usr/lib libgcc_s.so result2 start_clock load_to_memory stop_clock log

	# usunięcie zaalokowanych bloków o różnych rozmiarach 
	@ echo "test usuwania blokow z pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory / libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory /usr libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./main_dynamic create_table 3 search_directory /usr/lib libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log

	# na przemian  kilkakrotne dodanie i usunięcie zadanej liczby bloków 
	@ echo "test dodawania i usuwania zadanej liczby blokow" >> log
	@ $(PREFIX) ./main_dynamic create_table $(BLOCK_COUNT) search_directory / libgcc_s.so result2 \
	start_clock \
	$(COMMAND) \
	stop_clock \
	log

rebuild: clean build