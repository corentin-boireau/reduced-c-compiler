import os
import subprocess

# Weird trick to get ANSI escape sequences working on Windows
os.system("")
RED="\033[31m"
CYAN="\033[34m"
GREY="\033[90m"
RESET="\033[0m"
BOLD="\033[1m"

def to_success(str):
    return CYAN + str + RESET

def to_error(str):
    return RED + str + RESET

def to_skip(str):
    return GREY + str + RESET

def to_bold_success(str):
    return BOLD + CYAN + str + RESET

def to_bold_error(str):
    return BOLD + to_error(str)

def to_bold_skip(str):
    return BOLD + to_skip(str)

def are_files_equal(file_to_compare, reference_file):
    are_equal = False
    with open(file_to_compare, "r", errors="backslashreplace") as to_cmp_file, open(reference_file, "r", errors="backslashreplace") as ref_file:
        are_equal = to_cmp_file.read() == ref_file.read()

    return are_equal

def convert_test_nb_to_string(test_nb):
    return str(test_nb) if test_nb >= 100 else "0" + str(test_nb) if test_nb >= 10 else "00" + str(test_nb)
        

def test_compare_files(file_to_compare, reference_file, test_nb, skip_test=False):
    success = True

    test_nb_str = convert_test_nb_to_string(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : " + to_bold_skip("SK") + " : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
    else:
        print("[" + test_nb_str + "] : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\" ...", end="")

        if are_files_equal(file_to_compare, reference_file):
            print("\r[" + test_nb_str + "] : " + to_bold_success("OK") + " : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
        else:
            print("\r[" + test_nb_str + "] : " + to_bold_error("KO") + " : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
            success = False
    
    return success


def test_run_process(description, args, test_nb, out_filename, err_filename, in_filename=None, skip_test=False):
    success = True

    test_nb_str = convert_test_nb_to_string(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : " + to_skip("SK") + " : " +  description, end="\n")
    else:
        print("[" + test_nb_str + "] : " + description + " ...", end="")
        
        ret_code = 0
        if in_filename is None:
            with open(out_filename, "w") as out_file, open(err_filename, "w") as err_file:
                process = subprocess.Popen(args, env=os.environ, stdout=out_file, stderr=err_file)
                ret_code = process.wait()
        else:
            with open(in_filename, "r") as to_exec_file, open(out_filename, "w") as result_file, open(err_filename, "w") as err_file:
                process = subprocess.Popen(args, env=os.environ, stdin=to_exec_file, stdout=result_file, stderr=err_file)
                ret_code = process.wait()
        
        if ret_code == 0:
            print("\r[" + test_nb_str + "] : " + to_bold_success("OK") + " : " + description, end="\n")
        else:
            print("\r[" + test_nb_str + "] : " + to_bold_error("KO") + " : " + description, end="\n")
            success = False

    return success
