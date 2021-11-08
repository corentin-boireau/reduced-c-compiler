CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -O0 -g
LD_FLAGS=-lm
INC_DIR=ReducedCCompiler/vendor

SRC_DIR=ReducedCCompiler/src
BIN_DIR=bin/ReducedCCompiler/Debug-x64

PY=python3
TEST_DIR=ReducedCCompiler-Test
TEST_MAIN=ReducedCCompiler_Test.py

# Executed when the Makefile is parsed
$(shell mkdir -p "bin/MiniStackMachine/Debug-x64")
$(shell mkdir -p "bin/ReducedCCompiler/Debug-x64")

.PHONY: all
all: msm rcc

.PHONY: rebuild
rebuild: clean all

.PHONY: test
test: msm rcc
	@cd $(TEST_DIR); $(PY) $(TEST_MAIN)

.PHONY: extratest
extratest: msm rcc
	@cd $(TEST_DIR); $(PY) $(TEST_MAIN) extra

.PHONY: clean
clean:
	find . -regextype sed -regex ".*\.\(o\|txt\|msm\)" -delete
	rm -f $(BIN_DIR)/rcc bin/MiniStackMachine/Debug-x64/msm

.PHONY: rcc
rcc: $(BIN_DIR)/rcc

$(BIN_DIR)/rcc: $(BIN_DIR)/main.o $(BIN_DIR)/token.o $(BIN_DIR)/syntactic_node.o $(BIN_DIR)/syntactic_analysis.o $(BIN_DIR)/semantic_analysis.o $(BIN_DIR)/main.o $(BIN_DIR)/code_generation.o $(BIN_DIR)/argtable3.o
	$(CC) $^ $(LD_FLAGS) -o $@ 

$(BIN_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) -c $(CFLAGS) -I $(INC_DIR) $(SRC_DIR)/main.c -o $@

$(BIN_DIR)/token.o: $(SRC_DIR)/token.c $(SRC_DIR)/token.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/token.c -o $@

$(BIN_DIR)/syntactic_node.o: $(SRC_DIR)/syntactic_node.c $(SRC_DIR)/syntactic_node.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/syntactic_node.c -o $@

$(BIN_DIR)/syntactic_analysis.o: $(SRC_DIR)/syntactic_analysis.c $(SRC_DIR)/syntactic_analysis.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/syntactic_analysis.c -o $@

$(BIN_DIR)/semantic_analysis.o: $(SRC_DIR)/semantic_analysis.c $(SRC_DIR)/semantic_analysis.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/semantic_analysis.c -o $@

$(BIN_DIR)/code_generation.o: $(SRC_DIR)/code_generation.c $(SRC_DIR)/code_generation.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)/code_generation.c -o $@

$(BIN_DIR)/argtable3.o: $(INC_DIR)/argtable3/argtable3.c $(INC_DIR)/argtable3/argtable3.h
	$(CC) -c $(CFLAGS) $(INC_DIR)/argtable3/argtable3.c -o $@

.PHONY: msm
msm: bin/MiniStackMachine/Debug-x64/msm

bin/MiniStackMachine/Debug-x64/msm: MiniStackMachine/src/msm.c
	$(CC) -g $^ -o $@
