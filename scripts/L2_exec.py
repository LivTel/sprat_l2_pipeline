from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np
from optparse import OptionParser
import os
from subprocess import Popen, PIPE
from shutil import copyfile, move
import time

L2_BIN_DIR 	= os.environ['L2_BIN_DIR']
L2_TEST_DIR 	= os.environ['L2_TEST_DIR']
L2_SCRIPT_DIR	= os.environ['L2_SCRIPT_DIR']
L2_MAN_DIR	= os.environ['L2_MAN_DIR']

def print_header():
    with open(L2_MAN_DIR + "/HEADER") as f:
        for line in f:
	    print line.strip('\n')
    time.sleep(1)
    
def print_routine(routine):
    bar = []
    for i in range(len(routine)):
        bar.append("*")
    print ''.join(bar) + '****'
    print '* ' + routine + ' *'
    print ''.join(bar) + '****'
    time.sleep(1)    
    
def print_notification(message):
    print "* " + message
    print
    
if __name__ == "__main__":
  
    print_header()
    
    parser = OptionParser()
    parser.add_option('--t', dest='f_target', action='store', default=L2_TEST_DIR + "/target.fits")
    parser.add_option('--r', dest='f_ref', action='store', default=L2_TEST_DIR + "/ref.fits")
    parser.add_option('--c', dest='f_cont', action='store', default=L2_TEST_DIR + "/continuum.fits")
    parser.add_option('--a', dest='f_arc', action='store', default=L2_TEST_DIR + "/arc.fits")
    parser.add_option('--dir', dest='work_dir', action='store', default="test")
    parser.add_option('--o', dest='clobber', action='store_true')
    (options, args) = parser.parse_args()

    f_target = options.f_target
    f_ref = options.f_ref
    f_cont = options.f_cont
    f_arc = options.f_arc
    work_dir = options.work_dir
    clobber = options.clobber

    # input sanity checks
    if not all([f_target, f_ref, f_cont, f_arc]):
        print "Input files are undefined"
        exit(0)
    elif not os.path.exists(f_target):
        print "Target file doesn't exist"
        exit(0)    
    elif not os.path.exists(f_ref):
        print "Reference file doesn't exist"
        exit(0)    
    elif not os.path.exists(f_cont):	
        print "Continuum file doesn't exist"
        exit(0)    
    elif not os.path.exists(f_arc):	
        print "Arc file doesn't exist"
        exit(0)
    
    # define output extensions
    target 		= os.path.splitext(os.path.basename(f_target))[0]
    ref 		= os.path.splitext(os.path.basename(f_ref))[0]
    cont		= os.path.splitext(os.path.basename(f_cont))[0]
    arc			= os.path.splitext(os.path.basename(f_arc))[0]
    target_suffix 	= "_target"
    ref_suffix 		= "_ref"
    cont_suffix		= "_cont"
    arc_suffix 		= "_arc"
    trim_suffix 	= "_tr"
    cor_suffix		= "_cor"

    # define routine paths
    clip 	        = L2_BIN_DIR + "/spclip"
    find	        = L2_BIN_DIR + "/spfind"
    trace	        = L2_BIN_DIR + "/sptrace"
    correct	        = L2_BIN_DIR + "/spcorrect"

    #define script paths
    plot_peaks	        = L2_SCRIPT_DIR + "/L2_analyse_plotpeaks.py"
    
    # move files to working directory, redefine paths and change to working directory
    try:
        if not os.path.exists(work_dir):
            os.mkdir(work_dir)
            copyfile(f_target, work_dir + "/" + target + target_suffix + ".fits")
            copyfile(f_ref, work_dir + "/" + ref + ref_suffix + ".fits")
            copyfile(f_cont, work_dir + "/" + cont + cont_suffix + ".fits")
            copyfile(f_arc, work_dir + "/" + arc + arc_suffix + ".fits")    
        else:
            if clobber:
                for i in os.listdir(work_dir):
	            os.remove(work_dir + "/" + i)
                copyfile(f_target, work_dir + "/" + target + target_suffix + ".fits")
                copyfile(f_ref, work_dir + "/" + ref + ref_suffix + ".fits")
                copyfile(f_cont, work_dir + "/" + cont + cont_suffix + ".fits")
                copyfile(f_arc, work_dir + "/" + arc + arc_suffix + ".fits") 
            else:
	        print "Working directory is not empty"
	        exit(0)
    except OSError:
        print "Failed to copy files to working directory"
        exit(0)
    
    f_target = target + target_suffix + ".fits"
    f_ref = ref + ref_suffix + ".fits"
    f_cont = cont + cont_suffix + ".fits"
    f_arc = arc + arc_suffix + ".fits"

    os.chdir(work_dir)

    # -------------------------
    # - TRIM SPECTRA (SPTRIM) -
    # -------------------------
    # N.B. have to rewrite error_codes file after each run as 
    # each run gives the same header key.
    print_routine("Trim spectra (sptrim)")
    
    in_target_filename = f_target
    in_ref_filename = f_ref
    in_cont_filename = f_cont
    in_arc_filename = f_arc
    out_target_filename = target + target_suffix + trim_suffix + ".fits"
    out_ref_filename = ref + ref_suffix + trim_suffix + ".fits"
    out_cont_filename = cont + cont_suffix + trim_suffix + ".fits"
    out_arc_filename = arc + arc_suffix + trim_suffix + ".fits"

    output = Popen([clip, in_cont_filename, in_target_filename, "20", "0.1", "1.0", "3.0", "100", "1", "100", out_target_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCL"):
                    f_new.write("\n\n" + line + "\n\n")
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATCT\t" + code + "\t" + desc + " (target)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")	

    output = Popen([clip, in_cont_filename, in_ref_filename, "20", "0.1", "1.0", "3.0", "100", "1", "100", out_ref_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCL"):
                    f_new.write(line)
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATCR\t" + code + "\t" + desc + " (ref)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")		
		
    output = Popen([clip, in_cont_filename, in_cont_filename, "20", "0.1", "1.0", "3.0", "100", "1", "100", out_cont_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCL"):
                    f_new.write(line)
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATCC\t" + code + "\t" + desc + " (continuum)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")		
		
    output = Popen([clip, in_cont_filename, in_arc_filename, "20", "0.1", "1.0", "3.0", "100", "1", "100", out_arc_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCL"):
                    f_new.write(line)
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATCA\t" + code + "\t" + desc + " (arc)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")

    # ---------------------------------------------
    # - FIND PEAKS OF REFERENCE SPECTRUM (SPFIND) -
    # ---------------------------------------------
    print_routine("Find peaks of reference spectrum (spfind)")    
    in_ref_filename = ref + ref_suffix + trim_suffix + ".fits"

    output = Popen([find, in_ref_filename, "50", "0.1", "3", "3", "100", "4", "7", "3", "5"], stdout=PIPE)
    print output.stdout.read()  

    # ----------------------------------------------
    # - FIND SDIST OF REFERENCE SPECTRUM (SPTRACE) -
    # ----------------------------------------------
    print_routine("Find sdist of reference spectrum (sptrace)")      
    output = Popen([trace, "2"], stdout=PIPE)
    print output.stdout.read()  

    # ---------------------------------------------------------
    # - PLOT TRACE OF REFERENCE SPECTRUM PRE SDIST CORRECTION -
    # ---------------------------------------------------------
    print_routine("Plot trace of reference spectrum pre sdist correction")       
    in_ref_filename = ref + ref_suffix + trim_suffix + ".fits"

    output = Popen(["python", plot_peaks, "--f", in_ref_filename, "--o", "pre_sdist_cor.png", "--ot", "Pre SDIST correction"], stdout=PIPE)
    print output.stdout.read()   
    if os.path.exists("pre_sdist_cor.png"):
        print_notification("Success.")
    else:
        print_notification("Failed.")    

    # -----------------------------------------
    # - CORRECT SPECTRA FOR SDIST (SPCORRECT) -
    # -----------------------------------------
    # N.B. have to rewrite error_codes file after each run as 
    # each run gives the same header key.
    print_routine("Correct spectra for sdist (spcorrect)")      

    in_target_filename = target + target_suffix + trim_suffix + ".fits"
    in_ref_filename = ref + ref_suffix + trim_suffix + ".fits"
    in_cont_filename = cont + cont_suffix + trim_suffix + ".fits"
    in_arc_filename = arc + arc_suffix + trim_suffix + ".fits"

    out_target_filename = target + target_suffix + trim_suffix + cor_suffix + ".fits"
    out_ref_filename = ref + ref_suffix + trim_suffix + cor_suffix + ".fits"
    out_cont_filename = cont + cont_suffix + trim_suffix + cor_suffix + ".fits"
    out_arc_filename = arc + arc_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen([correct, in_target_filename, "linear", "1", out_target_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCO"):
                    f_new.write("\n\n" + line + "\n\n")
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATOT\t" + code + "\t" + desc + " (target)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")	

    output = Popen([correct, in_ref_filename, "linear", "1", out_ref_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCO"):
                    f_new.write(line)
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATOR\t" + code + "\t" + desc + " (ref)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")		
		
    output = Popen([correct, in_cont_filename, "linear", "1", out_cont_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCO"):
                    f_new.write(line)
                else:
		    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATOC\t" + code + "\t" + desc + " (continuum)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")		
		
    output = Popen([correct, in_arc_filename, "linear", "1", out_arc_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
	        if not line.startswith("L2STATCO"):
                    f_new.write(line)
                else:
                    key 	= line.split('\t')[0]
		    code 	= line.split('\t')[1]	
		    desc 	= line.split('\t')[2].strip('\n')	
		    f_new.write("L2STATOA\t" + code + "\t" + desc + " (arc)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")

    # ---------------------------------------------
    # - FIND PEAKS OF REFERENCE SPECTRUM (SPFIND) -
    # ---------------------------------------------
    print_routine("Find peaks of reference spectrum (spfind)")        
    in_ref_filename = ref + ref_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen([find, in_ref_filename, "50", "0.1", "3", "3", "100", "4", "7", "3", "5"], stdout=PIPE)
    print output.stdout.read()  

    # ----------------------------------------------
    # - FIND SDIST OF REFERENCE SPECTRUM (SPTRACE) -
    # ----------------------------------------------
    print_routine("Find sdist of reference spectrum (sptrace)")         
    output = Popen([trace, "2"], stdout=PIPE)
    print output.stdout.read()  

    # ------------------------------------------------
    # - PLOT TRACE OF SPECTRUM POST SDIST CORRECTION -
    # ------------------------------------------------
    print_routine("Plot trace of spectrum post sdist correction (l2pp)")     
    in_ref_filename = ref + ref_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen(["python", plot_peaks, "--f", in_ref_filename, "--o", "post_sdist_cor.png", "--ot", "Post SDIST correction"], stdout=PIPE)
    print output.stdout.read()  
    if os.path.exists("post_sdist_cor.png"):
        print_notification("Success.")
    else:
        print_notification("Failed.")


	
	
    

