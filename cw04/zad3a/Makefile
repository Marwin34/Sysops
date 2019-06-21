GCC=gcc -Wall
CATCHER_SRC=catcher.c
CATCHER_OUT=catcher
SENDER_SRC=sender.c
SENDER_OUT=sender


build: $(CATCHER_SRC) $(SENDER_SRC)
	$(GCC) $(CATCHER_SRC) -o $(CATCHER_OUT)
	$(GCC) $(SENDER_SRC) -o $(SENDER_OUT)

clean:
	rm -rf *.o

tests: $(CATCHER_OUT) $(SENDER_OUT) tests.sh
	@bash tests.sh kill
	@bash tests.sh sigqueue
	@bash tests.sh sigrt