MONITOR_SRC = monitor.c
MONITOR_OUT = monitor
TESTER_SRC = tester.c
TESTER_OUT = tester
ADDITIONAL_SRC = backup.c
GCC = gcc 

build: $(MONITOR_SRC) $(ADDITIONAL_SRC) $(TESTER_SRC)
	$(GCC) -Wall $(MONITOR_SRC) $(ADDITIONAL_SRC) -o $(MONITOR_OUT)
	$(GCC) -Wall $(TESTER_SRC) -o $(TESTER_OUT)

tests: lista archiwum $(MONITOR_OUT)
	# Test typu 0
	@./$(MONITOR_OUT) lista 10 0
	# Test typu 1
	@./$(MONITOR_OUT) lista 10 1

run_tester: $(TESTER_OUT)
	@./tester test 2 10 15