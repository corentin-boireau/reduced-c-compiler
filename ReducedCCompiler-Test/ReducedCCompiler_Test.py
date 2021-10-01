import os
from test_lexical_pkg import test_lexical

def run_all_tests():
    print("=== Reduced C Compiler test suite ===")
    os.chdir("test_lexical_pkg")
    test_lexical.test_lexical()

if __name__ == "__main__":
    run_all_tests()