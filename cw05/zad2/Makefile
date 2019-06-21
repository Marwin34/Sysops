GCC=gcc -Wall
MASTER_SRC=master.c
SLAVE_SRC=slave.c
MASTER_OUT=master
SLAVE_OUT=slave


build: $(MASTER_SRC) $(SLAVE_SRC)
	$(GCC) $(MASTER_SRC) -o $(MASTER_OUT)
	$(GCC) $(SLAVE_SRC) -o $(SLAVE_OUT)

clean:
	rm -rf $(MASTER_OUT) $(SLAVE_OUT)

test: $(MASTER_OUT) $(SLAVE_OUT)
	./$(MASTER_OUT) fifoo

run_slave_5: $(SLAVE_OUT)
	./$(SLAVE_OUT) fifoo 5

run_slave_10: $(SLAVE_OUT)
	./$(SLAVE_OUT) fifoo 10	