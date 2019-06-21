GCC=gcc -Wall
SRC=monitor.c backup.c
OUT=monitor
TESTER_SRC=tester.c
TESTER_OUT=tester


build: $(SRC) $(TESTER_SRC)
	$(GCC) $(SRC) -o $(OUT)
	$(GCC) $(TESTER_SRC) -o $(TESTER_OUT)

clean:
	rm -rf *.o

run_monitor: $(OUT) archiwum lista
	./monitor lista

run_tester: $(TESTER_OUT)
	./tester test1 1 5 20
