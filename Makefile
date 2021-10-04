
CC = gcc
C_FLAGS = -Wall -Wextra -ansi

RCC = rcc
RCC_SRC      = $(wildcard ReducedCCompiler/src/*.c)
RCC_SRC     += ReducedCCompiler/vendor/argtable3/argtable3.c
RCC_LD_FLAGS = -lm
RCC_INC_DIR  = ReducedCCompiler/vendor
RCC_EXE_DIR  = bin/ReducedCCompiler/Debug-x64

MSM = msm
MSM_SRC      = MiniStackMachine/src/msm.c
MSM_EXE_DIR  = bin/MiniStackMachine/Debug-x64

PY = python3
TEST_DIR = ReducedCCompiler-Test
TEST_ALL = ReducedCCompiler_Test.py


all: $(RCC) $(MSM)

clean:
	rm -f $(RCC_EXE_DIR)/$(RCC)
	rm -f $(MSM_EXE_DIR)/$(MSM)
	find . -name *.txt -exec rm {} \;

test: $(RCC) $(MSM)
	cd $(TEST_DIR); \
	   $(PY) $(TEST_ALL)

$(RCC):
	$(CC) -o $(RCC_EXE_DIR)/$(RCC) $(RCC_SRC) -I $(RCC_INC_DIR) $(RCC_LD_FLAGS)


$(MSM):
	$(CC) -o $(MSM_EXE_DIR)/$(MSM) $(MSM_SRC)

