GCC=gcc -Wall
INTERPRETER_SRC=interpreter.c
INTERPRETER_OUT=interpreter
COMMANDS=commands


build: $(INTERPRETER_SRC)
	$(GCC) $(INTERPRETER_SRC) -o $(INTERPRETER_OUT)

clean:
	rm -rf $(INTERPRETER_OUT) $(COMMANDS)

test: $(INTERPRETER_OUT) $(COMMANDS)
	./$(INTERPRETER_OUT) commands

test_commands:
	@touch commands
	@echo "ls -l | grep a | wc -l" >> commands
	@echo "ls -l | grep total | wc -l" >> commands
	@echo "ls -l | grep marcin | wc -l" >> commands
