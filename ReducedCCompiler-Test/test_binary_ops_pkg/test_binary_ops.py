import subprocess
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))

import tests_utils as tu

def test_binary_ops():
    LOG_DIR = "logs"

    FILE_PREFIXES = ["binary_ops"]
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
        test_nb_str = tu.convert_test_nb_to_string(test_nb)
        out_filename = LOG_DIR + "/out_" + test_nb_str + ".txt"
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
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

        # CODE GENERATION
        msm_output_filename = FILE_PREFIXES[test_file_nb] + MSM_EXT
        msm_ref_filename = msm_output_filename + REF_EXT

        args = [RCC_PATH, "--no-runtime",  test_filename, "-o", msm_output_filename]
        desc = "Compiling " + test_filename
        test_nb_str = tu.convert_test_nb_to_string(test_nb)
        out_filename = LOG_DIR + "/out_" + test_nb_str + ".txt"
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(msm_output_filename, msm_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        test_nb += 1

        # EXECUTION
        exec_output_filename = FILE_PREFIXES[test_file_nb] + EXEC_SUFFIX + OUT_EXT
        exec_input_filename = msm_output_filename
        exec_ref_filename = exec_output_filename + REF_EXT

        args = [MSM_PATH]
        desc = "Running " + msm_output_filename
        test_nb_str = tu.convert_test_nb_to_string(test_nb)
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
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

        # OPTIMISATION ON CONSTANTS
        opti_msm_output_filename = FILE_PREFIXES[test_file_nb] + OPTI_SUFFIX + MSM_EXT
        
        args = [RCC_PATH, "--no-runtime",  test_filename, "--opti-const-op", "-o", opti_msm_output_filename]
        desc = "Compiling " + test_filename + " with --opti-const-op flag"
        test_nb_str = tu.convert_test_nb_to_string(test_nb)
        out_filename = LOG_DIR + "/out_" + test_nb_str + ".txt"
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)
        
        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1

        opti_exec_output_filename = FILE_PREFIXES[test_file_nb] + OPTI_SUFFIX + EXEC_SUFFIX + OUT_EXT
        opti_exec_input_filename = opti_msm_output_filename

        args = [MSM_PATH]
        desc = "Running " + opti_msm_output_filename
        test_nb_str = tu.convert_test_nb_to_string(test_nb)
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
        success = tu.test_run_process(desc, args, test_nb,
                                      in_filename=opti_exec_input_filename,
                                      out_filename=opti_exec_output_filename,
                                      err_filename=err_filename,
                                      skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(opti_exec_output_filename, exec_output_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1

    return nb_errors


if __name__ == "__main__":
    print("Test binary operations")
    nb_errors = test_binary_ops()
    if nb_errors > 0:
        print(tu.to_bold_error("\nXXX " + str(nb_errors) + (" error" if nb_errors == 1 else " errors") + " XXX"))
    else:
        print(tu.to_bold_success("   All tests passed"))
