import filecmp
import os
import subprocess


def test_compare_files(file_to_compare, reference_file, test_nb, skip_test=False):
    success = True

    test_nb_str = str(test_nb) if test_nb < 10 else " " + str(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : SK : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
    else:
        print("[" + test_nb_str + "] : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\" ...", end="")

        if filecmp.cmp(file_to_compare, reference_file):
            print("\r[" + test_nb_str + "] : OK : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
        else:
            print("\r[" + test_nb_str + "] : KO : Comparing \"" + file_to_compare + "\" to \"" + reference_file + "\"", end="\n")
            success = False
    
    return success


def test_run_process(description, args, test_nb, out_filename, err_filename, skip_test=False):
    success = True

    test_nb_str = str(test_nb) if test_nb < 10 else " " + str(test_nb)

    if skip_test:
        print("[" + test_nb_str + "] : SK : " + description, end="\n")
    else:
        print("[" + test_nb_str + "] : " + description + " ...", end="")
        
        out_file = open(out_filename, "w")
        err_file = open(err_filename, "w")

        process = subprocess.Popen(args, env=os.environ, stdout=out_file, stderr=err_file)
        process.wait()

        out_file.close()
        err_file.close()
        
        if process.returncode == 0:
            print("\r[" + test_nb_str + "] : OK : " + description, end="\n")
        else:
            print("\r[" + test_nb_str + "] : KO : " + description, end="\n")
            success = False

    return success