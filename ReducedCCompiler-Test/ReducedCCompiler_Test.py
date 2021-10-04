import os
from test_lexical_pkg.test_lexical       import test_lexical
from test_unary_ops_pkg.test_unary_ops   import test_unary_ops
from test_binary_ops_pkg.test_binary_ops import test_binary_ops
from test_variables_pkg.test_variables   import test_variables
from test_conditions_pkg.test_conditions import test_conditions
from test_loops_pkg.test_loops           import test_loops
from test_loops_pkg.test_break_continue  import test_break_continue

def run_all_tests():
    nb_errors = 0
    print("=== Reduced C Compiler test suite ===\n")

    print("= Test lexical analysis =")
    os.chdir("test_lexical_pkg")
    nb_errors += test_lexical()
    os.chdir("..")

    print("\n= Test unary operators =")
    os.chdir("test_unary_ops_pkg")
    nb_errors += test_unary_ops()
    os.chdir("..")

    print("\n= Test binary operators =")
    os.chdir("test_binary_ops_pkg")
    nb_errors += test_binary_ops()
    os.chdir("..")

    print("\n= Test variables =")
    os.chdir("test_variables_pkg")
    nb_errors += test_variables()
    os.chdir("..")

    print("\n= Test conditions =")
    os.chdir("test_conditions_pkg")
    nb_errors += test_conditions()
    os.chdir("..")

    print("\n= Test loops =")
    os.chdir("test_loops_pkg")
    nb_errors += test_loops()
    os.chdir("..")

    print("\n= Test break & continue =")
    os.chdir("test_loops_pkg")
    nb_errors += test_break_continue()
    os.chdir("..")

    if nb_errors > 0:
        print("\nXXX " + str(nb_errors) + (" error" if nb_errors == 1 else "errors") + " XXX")


if __name__ == "__main__":
    run_all_tests()
