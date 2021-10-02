import tests_utils as tu
import os

def test_lexical():
    LOG_DIR = "logs"

    TEST_FILES = ["valid.c",
                  "mix.c",
                  "junk.c"]

    OUTPUT_FILES = ["valid.txt",
                    "mix.txt",
                    "junk.txt"]

    REFERENCE_FILES = ["valid.txt.ref",
                       "mix.txt.ref",
                       "junk.txt.ref"]

    assert len(TEST_FILES) == len(OUTPUT_FILES) and len(OUTPUT_FILES) == len(REFERENCE_FILES)

    RCC_PATH = "../../bin/ReducedCCompiler/Debug-x64/rcc"

    test_nb = 1
    nb_errors = 0
    skip_next = False

    if not os.path.isdir(LOG_DIR):
       os.mkdir(LOG_DIR)

    for test_file_nb in range(0, len(TEST_FILES)):
        args = [RCC_PATH, TEST_FILES[test_file_nb], "--stage", "lexical", "-o", OUTPUT_FILES[test_file_nb]]
        desc = "Running lexical analysis on " + TEST_FILES[test_file_nb]
        out_filename = LOG_DIR + "/out_" + str(test_nb) + ".txt"
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(OUTPUT_FILES[test_file_nb], REFERENCE_FILES[test_file_nb], test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1


if __name__ == "__main__":
    print("Test lexical")
    test_lexical()