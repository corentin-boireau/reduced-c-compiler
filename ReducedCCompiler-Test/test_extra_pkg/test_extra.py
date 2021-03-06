import subprocess
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(SCRIPT_DIR))

import tests_utils as tu

def test_extra():
    LOG_DIR = "logs"

    FILE_PREFIXES = [
        "bin_val", "decl_assign", "do_while", "do_while_break", "do_while_continue",
        "elseif", "for1", "for_break", "for_continue", "func_args", "func_call", "func_cond", "func_loop", "func_rec", "func_rec2", 
        "func_ret", "func_ret2", "hex_val", "loop_scope", "lots_do_while", "lots_for", "lots_scopes", "lots_var", "lots_while", "main",
        "multiple_assigment", "multiple_assigment2", "multiple_var", "mult_expr", "mult_line", "nestedif", "nested_do_while", 
        "nested_do_while_break", "nested_do_while_continue", "nested_for", "nested_for_break", "nested_for_continue", "nested_while", 
        "nested_while_break", "nested_while_continue", "oct_val", "prefix", "prefix2", "ptr1", "ptr2", "ptr3", "scope_var", "simple",
        "simple2", "simple3", "single_line", "while1", "while_break", "while_continue",
    ]
    TEST_EXT = ".c"
    MSM_EXT  = ".msm"
    OUT_EXT  = ".txt"
    REF_EXT  = ".ref"
    
    test_nb = 1
    nb_errors = 0
    skip_next = False

    if not os.path.isdir(LOG_DIR):
       os.mkdir(LOG_DIR)

    for test_file_nb in range(0, len(FILE_PREFIXES)):
        test_filename = FILE_PREFIXES[test_file_nb] + TEST_EXT

        # CODE GENERATION
        msm_output_filename = FILE_PREFIXES[test_file_nb] + MSM_EXT

        args = [tu.RCC_PATH, "--no-runtime",  test_filename, "-o", msm_output_filename]
        desc = "Compiling " + test_filename
        test_nb_str = str(test_nb) if test_nb >= 100 else "0" + str(test_nb) if test_nb >= 10 else "00" + str(test_nb)
        out_filename = LOG_DIR + "/out_" + test_nb_str + ".txt"
        err_filename = LOG_DIR + "/err_" + test_nb_str + ".txt"
        success = tu.test_run_process(desc, args, test_nb, out_filename=out_filename, err_filename=err_filename, skip_test=skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1

        # EXECUTION
        exec_output_filename = FILE_PREFIXES[test_file_nb] + OUT_EXT
        exec_input_filename = msm_output_filename
        exec_ref_filename = exec_output_filename + REF_EXT

        args = [tu.MSM_PATH]
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
    print("Extra tests")
    nb_errors = test_extra()
    if nb_errors > 0:
        print(tu.to_bold_error("\nXXX " + str(nb_errors) + (" error" if nb_errors == 1 else " errors") + " XXX"))
    else:
        print(tu.to_bold_success("   All tests passed"))
