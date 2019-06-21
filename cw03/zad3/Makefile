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
	@./$(MONITOR_OUT) lista 10 0 100 100
	# Test typu 1
	@./$(MONITOR_OUT) lista 10 1 150 100

run_tester: $(TESTER_OUT)
	@./$(TESTER_OUT) test 2 5 14