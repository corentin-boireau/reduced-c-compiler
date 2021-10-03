import os
from test_lexical_pkg import test_lexical
from test_unary_ops_pkg import test_unary_ops
from test_binary_ops_pkg import test_binary_ops

def run_all_tests():
    print("=== Reduced C Compiler test suite ===\n")

    print("= Test lexical analysis =")
    os.chdir("test_lexical_pkg")
    test_lexical.test_lexical()
    os.chdir("..")

    print("\n= Test unary operators =")
    os.chdir("test_unary_ops_pkg")
    test_unary_ops.test_unary_ops()
    os.chdir("..")

    print("\n= Test binary operators =")
    os.chdir("test_binary_ops_pkg")
    test_binary_ops.test_binary_ops()
    os.chdir("..")

if __name__ == "__main__":
    run_all_tests()