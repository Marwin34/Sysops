SRC := program.c
SRC_NFTW := program_nftw.c
OUT := program
OUT_NFTW := program_nftw

build: $(SRC) $(SRC_NFTW)
	gcc -Wall $(SRC) -o $(OUT)
	gcc -Wall $(SRC_NFTW) -o $(OUT_NFTW)

clean:
	rm *.o

create_hierarchy:
	#Tworzenie przykladowej hierarchi
	@./hierarchy.sh

delete_hierarchy: top
	rm -rf top

tests: $(OUT) $(OUT_NFTW) create_hierarchy
	#Test programu uzywajacego opendir, readdir, stat
	#Wypisanie plikow i folderow z data modyfikacji wieksza od 2019-02-13 14:56:00 
	@./program ./top ">" "2019-02-13 14:56:00"
	#Wypisanie plikow i folderow z data modyfikacji mniejsza od 2019-02-13 14:56:00
	@./program ./top "<" "2019-02-13 14:56:00"
	#Wypisanie plikow i folderow z data modyfikacji rowna 2019-02-13 14:56:00
	@./program ./top "=" "2019-02-13 14:56:00"
	#Test programu uzywajacego funkcji nftw
	#Wypisanie plikow i folderow z data modyfikacji wieksza od 2019-02-13 14:56:00 
	@./program_nftw ./top ">" "2019-02-13 14:56:00"
	#Wypisanie plikow i folderow z data modyfikacji mniejsza od 2019-02-13 14:56:00
	@./program_nftw ./top "<" "2019-02-13 14:56:00"
	#Wypisanie plikow i folderow z data modyfikacji rowna 2019-02-13 14:56:00
	@./program_nftw ./top "=" "2019-02-13 14:56:00"