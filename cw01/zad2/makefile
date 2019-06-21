CC = gcc
LIB = find_library.c
SRC = main.c
OUT = main
PREFIX = LD_LIBRARY_PATH=../zad1

build: $(FILES)
	# compile binary (with shared)
	$(CC) -Wall -I../zad1 -L../zad1 -lfind_library $(SRC) -o $(OUT)

clean:
	rm -f *.o core

BLOCK_COUNT = 10
REPEAT_COUNT = 10

LOAD_COMMAND = $(shell for VARIABLE in {1..$(BLOCK_COUNT)}; do echo "load_to_memory";done)
REMOVE_COMMAND = $(shell for VARIABLE in {1..$(BLOCK_COUNT)}; do echo "remove_block `expr $$VARIABLE - 1`";done)
COMMAND = $(shell for VARIABLE in {1..$(REPEAT_COUNT)}; do echo $(LOAD_COMMAND) $(REMOVE_COMMAND);done )
test:
	# przeprowadzenie przeszukania katalogów o roznych poziomach zaglebien i roznych liczbach zawartych plikow
	@ echo "test przeszukiwania katalogow" >> log
	@ echo "bardzo zagniezdzony plik" >> log
	@ $(PREFIX) ./$(OUT) start_clock search_directory / libgcc_s.so result2 stop_clock log
	@echo "srednio zagniezdzony plik" >> log
	@ $(PREFIX) ./$(OUT) start_clock search_directory /usr libgcc_s.so result2 stop_clock log 
	@ echo "nie zagniezdzony plik" >> log
	@ $(PREFIX) ./$(OUT) start_clock search_directory /usr/lib libgcc_s.so result2 stop_clock log

	# zapisanie w pamieci blokow o roznych rozmiarach
	@ echo "test zapisu w pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory / libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory /usr libgcc_s.so result2 start_clock load_to_memory stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory /usr/lib libgcc_s.so result2 start_clock load_to_memory stop_clock log

	# usuniecie zaalokowanych blokow o roznych rozmiarach 
	@ echo "test usuwania blokow z pamieci" >> log
	@ echo "duzy plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory / libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "sredni plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory /usr libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log 
	@ echo "maly plik" >> log
	@ $(PREFIX) ./$(OUT) create_table 3 search_directory /usr/lib libgcc_s.so result2 load_to_memory start_clock remove_block 0 stop_clock log
	
	# na przemian  kilkakrotne dodanie i usuniecie zadanej liczby blokow 
	@ echo "test dodawania i usuwania zadanej liczby blokow" >> log
	@ $(PREFIX) ./$(OUT) create_table $(BLOCK_COUNT) search_directory / libgcc_s.so result2 \
	start_clock \
	$(COMMAND) \
	stop_clock \
	log

rebuild: clean build
