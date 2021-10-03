import tests_utils as tu
import os
import subprocess

def test_unary_ops():
    LOG_DIR = "logs"

    FILE_PREFIXES    = ["unary_ops"]
    TEST_EXT         = ".c"
    SYNTACTIC_SUFFIX = "_syn"
    SEMANTIC_SUFFIX  = "_sem"
    MSM_EXT          = ".msm"
    EXEC_SUFFIX      = "_exec"
    
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
        test_filename = FILE_PREFIXES[test_file_nb] + TEST_EXT
        syn_output_filename = FILE_PREFIXES[test_file_nb] + SYNTACTIC_SUFFIX + OUT_EXT
        syn_ref_filename = syn_output_filename + REF_EXT

        args = [RCC_PATH, test_filename, "--stage", "syntactic", "-o", syn_output_filename]
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

        msm_output_filename = FILE_PREFIXES[test_file_nb] + MSM_EXT
        msm_ref_filename = msm_output_filename + REF_EXT

        args = [RCC_PATH, test_filename, "-o", msm_output_filename]
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

        # Test execution of the code
        exec_output_filename = FILE_PREFIXES[test_file_nb] + EXEC_SUFFIX + OUT_EXT
        exec_input_filename = msm_output_filename
        exec_ref_filename = exec_output_filename + REF_EXT

        args = [MSM_PATH]
        desc = "Running " + msm_output_filename
        err_filename = LOG_DIR + "/err_" + str(test_nb) + ".txt"
        success = fn(desc, args, test_nb, exec_input_filename, exec_output_filename, err_filename, skip_next)

        if not success:
            nb_errors += 1
            skip_next = True

        test_nb += 1
        success = tu.test_compare_files(exec_output_filename, exec_ref_filename, test_nb, skip_test=skip_next)

        if not success:
            nb_errors += 1

        skip_next = False
        test_nb += 1


if __name__ == "__main__":
    print("Test unary operations")
    test_unary_ops()


def fn(description, args, test_nb, in_filename, out_filename, err_filename, skip_test=False):
    success = True

    test_nb_str = str(test_nb) if test_nb < 10 else " " + str(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : SK : " + description, end="\n")
    else:
        print("[" + test_nb_str + "] : " + description + " ...", end="")
        
        with open(in_filename, "r") as to_exec_file, open(out_filename, "w") as result_file, open(err_filename, "w") as err_file:
            process = subprocess.Popen(args, env=os.environ, stdin=to_exec_file, stdout=result_file, stderr=err_file)
            process.wait()
        
        if process.returncode == 0:
            print("\r[" + test_nb_str + "] : OK : " + description, end="\n")
        else:
            print("\r[" + test_nb_str + "] : KO : " + description, end="\n")
            success = False

    return success