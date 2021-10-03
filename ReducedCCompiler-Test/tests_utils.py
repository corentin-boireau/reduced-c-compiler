import os
import subprocess

def are_files_equal(file_to_compare, reference_file):
    are_equal = False
    with open(file_to_compare, "r") as to_cmp_file, open(reference_file, "r") as ref_file:
        are_equal = to_cmp_file.read() == ref_file.read()

    return are_equal

def test_compare_files(file_to_compare, reference_file, test_nb, skip_test=False):
    success = True

    test_nb_str = str(test_nb) if test_nb < 10 else " " + str(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : SK : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
    else:
        print("[" + test_nb_str + "] : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\" ...", end="")

        if are_files_equal(file_to_compare, reference_file):
            print("\r[" + test_nb_str + "] : OK : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
        else:
            print("\r[" + test_nb_str + "] : KO : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
            success = False
    
    return success


def test_run_process(description, args, test_nb, out_filename, err_filename, in_filename=None, skip_test=False):
    success = True

    test_nb_str = str(test_nb) if test_nb < 10 else " " + str(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : SK : " + description, end="\n")
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
            print("\r[" + test_nb_str + "] : OK : " + description, end="\n")
        else:
            print("\r[" + test_nb_str + "] : KO : " + description, end="\n")
            success = False

    return success