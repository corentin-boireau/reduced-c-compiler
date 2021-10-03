import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))

import tests_utils as tu

def test_lexical():
    LOG_DIR = "logs"

    FILE_PREFIXES = ["valid",
                    "mix",
                    "junk"]

    TEST_EXT      = ".c"
    LEXICAL_EXT   = "_lex"
    OUT_EXT       = ".txt"
    REF_EXT       = ".ref"

    RCC_PATH = "../../bin/ReducedCCompiler/Debug-x64/rcc"

    test_nb = 1
    nb_errors = 0
    skip_next = False

    if not os.path.isdir(LOG_DIR):
       os.mkdir(LOG_DIR)

    for test_file_nb in range(0, len(FILE_PREFIXES)):
        test_filename = FILE_PREFIXES[test_file_nb] + TEST_EXT
        lex_output_filename = FILE_PREFIXES[test_file_nb] + LEXICAL_EXT + OUT_EXT
        lex_ref_filename = lex_output_filename + REF_EXT

        args = [RCC_PATH, test_filename, "--stage", "lexical", "-o", lex_output_filename]
        desc = "Running lexical analysis on " + test_filename
        out_filename = LOG_DIR + "/out_" + str(test_nb) + ".txt"
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(lex_output_filename, lex_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1


if __name__ == "__main__":
    print("Test lexical")
    test_lexical()