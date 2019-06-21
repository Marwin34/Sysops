SRC := program.c
OUT := program

build: $(SRC)
	gcc -Wall $(SRC) -o $(OUT)

clean:
	rm *.o

tests: $(OUT)
	#Testy systemowej implementacji
	#Test dla 1 bajta
	@echo "TEST SYSTEMOWEJ IMPLEMENTACJI" >> wyniki.txt
	@echo "Test 1 bajta" >> wyniki.txt
	@./program generate test 5000 1
	@./program generate test2 10000 1
	@./program start_clock copy test test_copy 5000 1 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 1 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 1 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 1 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 4 bajtow
	@echo "Test 4 bajtow" >> wyniki.txt
	@./program generate test 5000 4
	@./program generate test2 10000 4
	@./program start_clock copy test test_copy 5000 4 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 4 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 4 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 4 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 512 bajtow
	@echo "Test 512 bajtow" >> wyniki.txt
	@./program generate test 5000 512
	@./program generate test2 10000 512
	@./program start_clock copy test test_copy 5000 512 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 512 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 512 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 512 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 1024 bajtow
	@echo "Test 1024 bajtow" >> wyniki.txt
	@./program generate test 5000 1024
	@./program generate test2 10000 1024
	@./program start_clock copy test test_copy 5000 1024 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 1024 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 1024 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 1024 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 4096 bajtow
	@echo "Test 4096 bajtow" >> wyniki.txt
	@./program generate test 5000 4096
	@./program generate test2 10000 4096
	@./program start_clock copy test test_copy 5000 4096 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 4096 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 4096 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 4096 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 8192 bajtow
	@echo "Test 8192 bajtow" >> wyniki.txt
	@./program generate test 5000 8192
	@./program generate test2 10000 8192
	@./program start_clock copy test test_copy 5000 8192 sys stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 8192 sys stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 8192 sys stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 8192 sys stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Testy bibliotecznej implementacji
	#Test dla 1 bajta
	@echo "TEST BIBLIOTECZNEJ IMPLEMENTACJI" >> wyniki.txt
	@echo "Test 1 bajta" >> wyniki.txt
	@./program generate test 5000 1
	@./program generate test2 10000 1
	@./program start_clock copy test test_copy 5000 1 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 1 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 1 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 1 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 4 bajtow
	@echo "Test 4 bajtow" >> wyniki.txt
	@./program generate test 5000 4
	@./program generate test2 10000 4
	@./program start_clock copy test test_copy 5000 4 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 4 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 4 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 4 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 512 bajtow
	@echo "Test 512 bajtow" >> wyniki.txt
	@./program generate test 5000 512
	@./program generate test2 10000 512
	@./program start_clock copy test test_copy 5000 512 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 512 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 512 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 512 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 1024 bajtow
	@echo "Test 1024 bajtow" >> wyniki.txt
	@./program generate test 5000 1024
	@./program generate test2 10000 1024
	@./program start_clock copy test test_copy 5000 1024 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 1024 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 1024 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 1024 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 4096 bajtow
	@echo "Test 4096 bajtow" >> wyniki.txt
	@./program generate test 5000 4096
	@./program generate test2 10000 4096
	@./program start_clock copy test test_copy 5000 4096 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 4096 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 4096 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 4096 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy

	#Test dla 8192 bajtow
	@echo "Test 8192 bajtow" >> wyniki.txt
	@./program generate test 5000 8192
	@./program generate test2 10000 8192
	@./program start_clock copy test test_copy 5000 8192 lib stop_clock log "Pomiar kopiowania 5000 blokow."
	@./program start_clock copy test2 test2_copy 10000 8192 lib stop_clock log "Pomiar kopiowania 10000 blokow."
	@./program start_clock sort test_copy 5000 8192 lib stop_clock log "Pomiar sortowania 5000 blokow."
	@./program start_clock sort test2_copy 10000 8192 lib stop_clock log "Pomiar sortowania 10000 blokow."
	@rm test test2 test_copy test2_copy