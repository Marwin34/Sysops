TRUCKER_SRC=trucker.c
LOADER_SRC=laoder.c
LOADER_MANAGER_SRC=loader_manager.c
TRUCKER_OUT=trucker
LOADER_OUT=loader
LOADER_MANAGER_OUT=loader_manager
GCC=gcc -Wall

build: $(TRUCKER_SRC) $(LOADER_SRC) $(LOADER_MANAGER_SRC)
	$(GCC) $(TRUCKER_SRC) -o $(TRUCKER_OUT)
	$(GCC) $(LOADER_SRC) -o $(LOADER_OUT)
	$(GCC) $(LOADER_MANAGER_SRC) -o $(LOADER_MANAGER_OUT)

run_trucker:
	./$(TRUCKER_OUT) 10 20 5

run_loader:
	./$(LOADER_OUT)	5

run_loader_manager:
	./$(LOADER_MANAGER_OUT) 3 5

clean:
	rm -rf $(TRUCKER_OUT) $(LOADER_OUT) $(LOADER_MANAGER_OUT)