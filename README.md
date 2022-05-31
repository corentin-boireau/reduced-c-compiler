# Reduced C Compiler

Reduced C Compiler is a compiler written in C for a subset of the C language (it only supports the `int` type).

At the moment it only targets a stack machine emulator called Mini Stack Machine that takes as input assembly code and interprets it directly.

A runtime is also provided with some basic functions for input/output and memory management :
- `malloc()` and `free()` to dynamically allocate or release memory on the heap
- `printn()` to print an integer on the standard ouput
- `scann()` to read an integer from the standard input
- `putchar()` and `getchar()` I/O primitives

## Setup

###  Building

- ####  On Windows

	**Prerequisites** :

	- Visual Studio 2019

	Open the solution file `ReducedCCompiler.sln` in Visual Studio and build it with `Ctrl + Shift + b`.

- #### On Linux

	**Prerequisites** :
  - CMake 3.22.1
  - gcc 11.2.0

  Run `cmake -S . -B bin && cmake --build bin` in the root directory.

### Environnment setup

It is recommended to add `bin/ReducedCCompiler/Debug-x64` and `bin/MiniStackMachine/Debug-x64` to your `PATH` to use easily `rcc` and `msm` executables.

It is also recommended to set an environnement variable `RCC_RUNTIME` containing `ReducedCCompiler/runtime.c`.

:warning: Given paths are relative to the repository's root but the corresponding absolute paths are required.
___

##  Running Tests

**Prerequisites** :
  - Python 3.7

On Windows, open the `ReducedCCompiler-Test` project in Visual Studio and run it without debugging `Ctrl + F5` (to avoid buggy display).

On Linux, run `make test` in the root directory.
___

## Usage
```
Reduced C Compiler.

Usage: rcc [-vh] <file> [-o <file>] [--no-runtime] [--runtime=<file>] [--stage=<lexical|syntactical|semantic>] [--no-const-fold] [--version]
  <file>                                   input file
  -o, --output=<file>                      output file
  -v, --verbose                            verbose output
  --no-runtime                             no runtime
  --runtime=<file>                         runtime file, default to environnment variable RCC_RUNTIME
  --stage=<lexical|syntactical|semantic>   stop the compilation at this stage
  --no-const-fold                          disable constant folding
  -h, --help                               display this help and exit
  --version                                display version info and exit
```
### Example

- Compile `hello.c` and write the generated assembly in `hello.msm` then run it with the emulator :
```
rcc hello.c -o hello.msm
msm hello.msm
# Or to compile and run with no output file
rcc hello.c | msm
```
- To visualize a particular stage of the compilation use the `--stage` option.
  With this file `test.c`
```
int a;

int main()
{
    a = 4;
    int b = a - 8;
    if (a < b)
        a = b;
}
```
It is possible to visualize the syntactic tree just after the semantic analysis stage with :
```
rcc test.c --stage=semantic --no-runtime
```
Output :

```
PROGRAM
|--SEQUENCE
   |--DECL : name = a, index = 0
|--FUNCTION : name = main
   |--SEQUENCE
   |--SEQUENCE
      |--DROP
         |--ASSIGNEMENT
            |--REF : name = a, index = 0 (global)
            |--CONST : value = 4
      |--SEQUENCE
         |--DECL : name = b, index = 0
         |--DROP
            |--ASSIGNEMENT
               |--REF : name = b, index = 0 (local)
               |--SUB
                  |--REF : name = a, index = 0 (global)
                  |--CONST : value = 8
      |--CONDITION
         |--LESS
            |--REF : name = a, index = 0 (global)
            |--REF : name = b, index = 0 (local)
         |--DROP
            |--ASSIGNEMENT
               |--REF : name = a, index = 0 (global)
               |--REF : name = b, index = 0 (local)
```
