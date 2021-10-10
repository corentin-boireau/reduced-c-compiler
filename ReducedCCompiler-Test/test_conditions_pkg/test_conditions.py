import subprocess
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))

import tests_utils as tu

def test_conditions():
    LOG_DIR = "logs"

    FILE_PREFIXES = ["simple_if", "simple_else"]
    TEST_EXT         = ".c"
    SYNTACTIC_SUFFIX = "_syn"
    SEMANTIC_SUFFIX  = "_sem"
    MSM_EXT          = ".msm"
    EXEC_SUFFIX      = "_exec"
    OPTI_SUFFIX      = "_opti"
    
    OUT_EXT       = ".txt"
    REF_EXT       = ".ref"
    
    OUT_EXT       = ".txt"
    REF_EXT       = ".ref"

    RCC_PATH = "../../bin/ReducedCCompiler/Debug-x64/rcc"
    MSM_PATH = "../../bin/MiniStackMachine/Debug-x64/msm"

    test_nb = 1
    nb_errors = 0
    skip_next = False

    if not os.path.isdir(LOG_DIR):
       os.mkdir(LOG_DIR)

    for test_file_nb in range(0, len(FILE_PREFIXES)):

        # SYNTACTIC ANALYSIS
        test_filename = FILE_PREFIXES[test_file_nb] + TEST_EXT
        syn_output_filename = FILE_PREFIXES[test_file_nb] + SYNTACTIC_SUFFIX + OUT_EXT
        syn_ref_filename = syn_output_filename + REF_EXT

        args = [RCC_PATH, "--no-runtime",  test_filename, "--stage", "syntactic", "-o", syn_output_filename]
        desc = "Running syntactical analysis on " + test_filename
        out_filename = LOG_DIR + "/out_" + str(test_nb) + ".txt"
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(syn_output_filename, syn_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1

        # SEMANTIC ANALYSIS
        test_filename = FILE_PREFIXES[test_file_nb] + TEST_EXT
        sem_output_filename = FILE_PREFIXES[test_file_nb] + SEMANTIC_SUFFIX + OUT_EXT
        sem_ref_filename = sem_output_filename + REF_EXT

        args = [RCC_PATH, "--no-runtime",  test_filename, "--stage", "semantic", "-o", sem_output_filename]
        desc = "Running syntactical analysis on " + test_filename
        out_filename = LOG_DIR + "/out_" + str(test_nb) + ".txt"
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(sem_output_filename, sem_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1

        # CODE GENERATION
        msm_output_filename = FILE_PREFIXES[test_file_nb] + MSM_EXT
        msm_ref_filename = msm_output_filename + REF_EXT

        args = [RCC_PATH, "--no-runtime",  test_filename, "-o", msm_output_filename]
        desc = "Compiling " + test_filename
        out_filename = LOG_DIR + "/out_" + str(test_nb) + ".txt"
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(msm_output_filename, msm_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1

        # EXECUTION
        exec_output_filename = FILE_PREFIXES[test_file_nb] + EXEC_SUFFIX + OUT_EXT
        exec_input_filename = msm_output_filename
        exec_ref_filename = exec_output_filename + REF_EXT

        args = [MSM_PATH]
        desc = "Running " + msm_output_filename
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = tu.test_run_process(desc, args, test_nb,
                                      in_filename=exec_input_filename,
                                      out_filename=exec_output_filename,
                                      err_filename=err_filename,
                                      skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(exec_output_filename, exec_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1

    return nb_errors


if __name__ == "__main__":
    print("Test conditions")
    nb_errors = test_conditions()
    if nb_errors > 0:
        print("XXX " + str(nb_errors) + (" error" if nb_errors == 1 else " errors") + " XXX")