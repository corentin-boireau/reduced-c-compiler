CC =gcc
CFLAGS =-Wall -Wextra -std=c99

RCC = rcc
RCC_BIN_DIR  = bin/ReducedCCompiler/Debug-x64
RCC_SRC      = $(wildcard ReducedCCompiler/src/*.c) \
			   ReducedCCompiler/vendor/argtable3/argtable3.c
RCC_OBJ      = $(notdir $(RCC_SRC:.c=.o))
RCC_LD_FLAGS = -lm
RCC_INC_DIR  = ReducedCCompiler/vendor

MSM = msm
MSM_SRC      = MiniStackMachine/src/msm.c
MSM_BIN_DIR  = bin/MiniStackMachine/Debug-x64

PY = python3
TEST_DIR = ReducedCCompiler-Test
TEST_ALL = ReducedCCompiler_Test.py


all: $(RCC) $(MSM)

clean:
	rm -f $(RCC_BIN_DIR)/$(RCC)
	rm -f $(MSM_BIN_DIR)/$(MSM)
	find . -name *.txt -exec rm {} \;
	find . -name *.msm -exec rm {} \;

test: $(RCC) $(MSM)
	cd $(TEST_DIR); \
	   $(PY) $(TEST_ALL)

$(RCC): $(RCC_SRC)
	$(CC) -o $(RCC_BIN_DIR)/$(RCC) $(RCC_SRC) $(CFLAGS) -I $(RCC_INC_DIR) $(RCC_LD_FLAGS)

#$(RCC_OBJ): $(RCC_SRC) 
#	$(CC) -o $(RCC_BIN_DIR)/$(RCC_OBJ) $(RCC_SRC) $(CFLAGS) -I $(RCC_INC_DIR)
#obj:
#	echo $(RCC_OBJ)

$(MSM): $(MSM_SRC)
	$(CC) -o $(MSM_BIN_DIR)/$(MSM) $(MSM_SRC)
