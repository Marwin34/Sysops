TRUCKER_SRC=trucker.c
LOADER_SRC=laoder.c
LOADER_MANAGER_SRC=loader_manager.c
TRUCKER_OUT=trucker
LOADER_OUT=loader
LOADER_MANAGER_OUT=loader_manager
MY_QUEUE_SRC=my_queue.c
GCC=gcc -Wall
LIB=-lrt -lpthread

build: $(TRUCKER_SRC) $(LOADER_SRC) $(LOADER_MANAGER_SRC) $(MY_QUEUE_SRC) $(LIB)
	$(GCC) $(TRUCKER_SRC) $(MY_QUEUE_SRC) -o $(TRUCKER_OUT) $(LIB)
	$(GCC) $(LOADER_SRC) $(MY_QUEUE_SRC) -o $(LOADER_OUT) $(LIB)
	$(GCC) $(LOADER_MANAGER_SRC) $(MY_QUEUE_SRC) -o $(LOADER_MANAGER_OUT) $(LIB)

run_trucker:
	./$(TRUCKER_OUT) 10 20 5

run_loader:
	./$(LOADER_OUT)

run_loader_manager:
	./$(LOADER_MANAGER_OUT) 3

clean:
	rm -rf $(TRUCKER_OUT) $(LOADER_OUT) $(LOADER_MANAGER_OUT)

