from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np
from optparse import OptionParser
import os
from subprocess import Popen, PIPE
from shutil import copyfile, move
import time
from datetime import date

L2_BIN_DIR 	= os.environ['L2_BIN_DIR']
L2_TEST_DIR 	= os.environ['L2_TEST_DIR']
L2_SCRIPT_DIR	= os.environ['L2_SCRIPT_DIR']
L2_MAN_DIR	= os.environ['L2_MAN_DIR']
L2_CONFIG_DIR   = os.environ['L2_CONFIG_DIR']

def print_header():
    with open(L2_MAN_DIR + "/HEADER") as f:
        for line in f:
	    print line.strip('\n')
    
def print_routine(routine):
    bar = []
    for i in range(len(routine)):
        bar.append("*")
    print ''.join(bar) + '****'
    print '* ' + routine + ' *'
    print ''.join(bar) + '****' 
    
def print_notification(message):
    print "* " + message
    print
    
if __name__ == "__main__":
  
    print_header()
    
    parser = OptionParser()
    parser.add_option('--t', dest='f_target', action='store', default=L2_TEST_DIR + "/default/target.fits", help="path to target file")
    parser.add_option('--r', dest='f_ref', action='store', default=L2_TEST_DIR + "/default/ref.fits", help="path to reference file")
    parser.add_option('--c', dest='f_cont', action='store', default=L2_TEST_DIR + "/default/continuum.fits", help="path to continuum file")
    parser.add_option('--a', dest='f_arc', action='store', default=L2_TEST_DIR + "/default/arc.fits", help="path to arc file")
    parser.add_option('--dir', dest='work_dir', action='store', default="test", help="path to working dir")
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
        exit(1)
    elif not os.path.exists(f_target):
        print "Target file doesn't exist"
        exit(1)    
    elif not os.path.exists(f_ref):
        print "Reference file doesn't exist"
        exit(1)    
    elif not os.path.exists(f_cont):	
        print "Continuum file doesn't exist"
        exit(1)    
    elif not os.path.exists(f_arc):	
        print "Arc file doesn't exist"
        exit(1)
    
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
    reb_suffix          = "_reb"
    ext_suffix          = "_ex"
    ss_suffix           = "_ss"

    # define routine paths
    clip 	        = L2_BIN_DIR + "/spclip"
    find	        = L2_BIN_DIR + "/spfind"
    trace	        = L2_BIN_DIR + "/sptrace"
    correct	        = L2_BIN_DIR + "/spcorrect"
    arcfit              = L2_BIN_DIR + "/sparcfit"
    extract             = L2_BIN_DIR + "/spextract"    
    rebin               = L2_BIN_DIR + "/sprebin"
    reformat		= L2_BIN_DIR + "/spreformat"
    
    # define output plot filenames
    ref_pre_sdist_plot  	= "ref_pre_sdist_plot.png"
    ref_post_sdist_plot 	= "ref_post_sdist_plot.png"
    target_output_L1_IMAGE	= "target_L1_IMAGE.png"
    target_output_SPEC_NONSS	= "target_SPEC_NONSS.png"
    target_output_SPEC_SS       = "target_SPEC_SS.png"

    # define script paths
    plot_peaks	        = L2_SCRIPT_DIR + "/L2_analyse_plotpeaks.py"
    plot_image		= L2_SCRIPT_DIR + "/L2_analyse_plotimage.py"
    plot_spec		= L2_SCRIPT_DIR + "/L2_analyse_plotspec.py"
    
    # define some wavelength fitting parameters
    start_wav           = 4020          # A
    end_wav             = 7960          # A
    dispersion          = 4.6           # A/px
    
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
	        exit(1)
    except OSError:
        print "Failed to copy files to working directory"
        exit(1)
    
    f_target = target + target_suffix + ".fits"
    f_ref = ref + ref_suffix + ".fits"
    f_cont = cont + cont_suffix + ".fits"
    f_arc = arc + arc_suffix + ".fits"

    os.chdir(work_dir)
    
    # determine appropriate arc list
    f_arc_fits = pyfits.open(f_arc)
    try:
        f_arc_fits_hdr_GRATROT = f_arc_fits[0].header['GRATROT']
        f_arc_fits_hdr_DATEOBS = f_arc_fits[0].header['DATE-OBS']
    except KeyError:
        print "Failed to find GRATROT key"
        f_arc_fits.close()
        exit(1)
    f_arc_fits.close()        
        
    if f_arc_fits_hdr_GRATROT == 0:     # red
        cfg = "red"
    else:
        cfg = "blue"
    path = L2_CONFIG_DIR + "/lookup_tables/" + cfg + "/arc.tab"
   
    a_path = []
    a_from_date = []
    a_from_time = []
    a_to_date = []
    a_to_time = []
    with open(path) as f:
        for line in f:
            if line != "\n":
                this_path = line.split('\t')[0].strip('\n').strip()
                this_from_date = line.split('\t')[1].strip('\n').strip()
                this_from_time = line.split('\t')[2].strip('\n').strip()
                a_path.append(this_path)
                a_from_date.append(this_from_date)
                a_from_time.append(this_from_time)
                
                this_to_date = line.split('\t')[3].strip('\n').strip()
                if ("now" in this_to_date):
                    today = date.today()
                    this_to_date = today.strftime("%d/%m/%y")
                    this_to_time = time.strftime("%H:%M:%S")
                else:
                    this_to_date = line.split('\t')[3].strip('\n').strip()
                    this_to_time = line.split('\t')[4].strip('\n').strip()
                
                a_to_date.append(this_to_date)
                a_to_time.append(this_to_time)
                 
    this_arc_datetime = time.strptime(f_arc_fits_hdr_DATEOBS, "%Y-%m-%dT%H:%M:%S.%f")
  
    chosen_arc_file_path = None
    for i in range(len(a_path)):
        this_from_time = time.strptime(a_from_date[i] + " " + a_from_time[i], "%d/%m/%y %H:%M:%S")
        this_to_time = time.strptime(a_to_date[i] + " " + a_to_time[i], "%d/%m/%y %H:%M:%S")
        
        if this_arc_datetime >= this_from_time and this_arc_datetime <= this_to_time:
            chosen_arc_file_path = L2_CONFIG_DIR + "/reference_arcs/" + cfg + "/" + a_path[i]
            break

    if chosen_arc_file_path is None:
        print "Failed to find a suitable arc"
        exit(1)        
        
    # add L2DATE key to additional_keys
    with open("additional_keys", 'w') as f:
        today = date.today()
        now_date = today.strftime("%d-%m-%y")
        now_time = time.strftime("%H:%M:%S")
        f.write("str\tSTARTDATE\tL2DATE\t" + now_date + " " + now_time + "\twhen this reduction was performed\n")

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
                    f_new.write(line)
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

    output = Popen([find, in_ref_filename, "50", "0.1", "3", "3", "100", "4", "50", "150", "7", "3", "5"], stdout=PIPE)
    print output.stdout.read()   
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATFI"):
                    f_new.write(line)
                else:
                    key         = line.split('\t')[0]
                    code        = line.split('\t')[1]   
                    desc        = line.split('\t')[2].strip('\n')       
                    f_new.write("L2STATF1\t" + code + "\t" + desc + " (ref uncorrected)" + "\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")    

    # ----------------------------------------------
    # - FIND SDIST OF REFERENCE SPECTRUM (SPTRACE) -
    # ----------------------------------------------
    print_routine("Find sdist of reference spectrum (sptrace)")      
    output = Popen([trace, "2"], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATTR"):
                    f_new.write(line)
                else:
                    key         = line.split('\t')[0]
                    code        = line.split('\t')[1]   
                    desc        = line.split('\t')[2].strip('\n')       
                    f_new.write("L2STATT1\t" + code + "\t" + desc + " (ref uncorrected)" + "\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")  

    # ---------------------------------------------------------
    # - PLOT TRACE OF REFERENCE SPECTRUM PRE SDIST CORRECTION -
    # ---------------------------------------------------------
    print_routine("Plot trace of reference spectrum pre sdist correction")       
    in_ref_filename = ref + ref_suffix + trim_suffix + ".fits"

    output = Popen(["python", plot_peaks, "--f", in_ref_filename, "--o", ref_pre_sdist_plot, "--ot", "Reference pre SDIST correction"], stdout=PIPE)
    print output.stdout.read()   
    if os.path.exists(ref_pre_sdist_plot):
        print_notification("Success.")
    else:
        print_notification("Failed.") 
        exit(1)

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
                    f_new.write(line)
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
    
    # --------------------
    # - RENAME DAT FILES -
    # --------------------   
    for i in os.listdir("."):
        if i.endswith(".dat"):
            if not i.startswith("p_"):
                filename = os.path.splitext(os.path.basename(i))[0]
                ext = os.path.splitext(os.path.basename(i))[1]
                move(i, "p_" + filename + "_ref_uncorrected" + ext)   

    # ---------------------------------------------
    # - FIND PEAKS OF REFERENCE SPECTRUM (SPFIND) -
    # ---------------------------------------------
    print_routine("Find peaks of reference spectrum (spfind)")        
    in_ref_filename = ref + ref_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen([find, in_ref_filename, "50", "0.1", "3", "3", "100", "4", "50", "150", "7", "3", "5"], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATFI"):
                    f_new.write(line)
    os.remove("error_codes")
    move("new_error_codes", "error_codes")      

    # ----------------------------------------------
    # - FIND SDIST OF REFERENCE SPECTRUM (SPTRACE) -
    # ----------------------------------------------
    print_routine("Find sdist of reference spectrum (sptrace)")         
    output = Popen([trace, "2"], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATTR"):
                    f_new.write(line)
    os.remove("error_codes")
    move("new_error_codes", "error_codes")    

    # ----------------------------------------------------------------------------------------------------------
    # - PLOT TRACE OF SPECTRUM POST SDIST CORRECTION AND CHECK RETURN CODE FOR SIGNIFICANT REMAINING CURVATURE -
    # ----------------------------------------------------------------------------------------------------------
    print_routine("Plot trace of spectrum post sdist correction (l2pp)")     
    in_ref_filename = ref + ref_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen(["python", plot_peaks, "--f", in_ref_filename, "--o", ref_post_sdist_plot, "--ot", "Reference post SDIST correction", "--c", "1.0"], stdout=PIPE)
    print output.stdout.read()  
    output.wait()
    if os.path.exists(ref_pre_sdist_plot) and output.returncode == 0:
        print_notification("Success.")
    else:
        print_notification("Failed.")
        exit(1)
           
    # --------------------
    # - RENAME DAT FILES -
    # --------------------   
    for i in os.listdir("."):
        if i.endswith(".dat"):
            if not i.startswith("p_"):
                filename = os.path.splitext(os.path.basename(i))[0]
                ext = os.path.splitext(os.path.basename(i))[1]
                move(i, "p_" + filename + "_ref_corrected" + ext)      
                
    # -------------------------------------------------
    # - FIND PIXEL TO WAVELENGTH SOLUTIONS (sparcfit) -
    # -------------------------------------------------
    print_routine("Find dispersion solution (sparcfit)")        
    in_arc_filename = arc + arc_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen([arcfit, in_arc_filename, "7", "7", "1", "2", chosen_arc_file_path, "3", "3", "100", "4"], stdout=PIPE)
    print output.stdout.read()           
    
    # -------------------
    # - REBIN (sprebin) -
    # -------------------
    print_routine("Rebin data spectrally (sprebin)")        
    in_target_filename = target + target_suffix + trim_suffix + cor_suffix + ".fits"
    out_target_filename = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ".fits"

    output = Popen([rebin, in_target_filename, str(start_wav), str(end_wav), "linear", str(dispersion), "1", out_target_filename], stdout=PIPE)
    print output.stdout.read()       
 
    # --------------------
    # - RENAME DAT FILES -
    # --------------------   
    for i in os.listdir("."):
        if i.endswith(".dat"):
            if not i.startswith("p_"):
                filename = os.path.splitext(os.path.basename(i))[0]
                ext = os.path.splitext(os.path.basename(i))[1]
                move(i, "p_" + filename + "_arc_corrected" + ext)      
                
    # ---------------------------------------------------------------
    # - FIND POSITION OF TARGET SPECTRUM WITH A SINGLE BIN (SPFIND) -
    # ---------------------------------------------------------------
    print_routine("Find peaks of target spectrum (spfind)")        
    in_target_filename = target + target_suffix + trim_suffix + cor_suffix + ".fits"

    output = Popen([find, in_target_filename, "1000", "0.1", "3", "3", "3", "4", "110", "140", "7", "3", "1"], stdout=PIPE)
    print output.stdout.read() 
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATFI"):
                    f_new.write(line)
                else:
                    key         = line.split('\t')[0]
                    code        = line.split('\t')[1]   
                    desc        = line.split('\t')[2].strip('\n')       
                    f_new.write("L2STATF2\t" + code + "\t" + desc + " (target corrected)"+ "\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")        
    
    # -------------------------------
    # - EXTRACT SPECTRA (SPEXTRACT) -
    # -------------------------------
    # N.B. have to rewrite error_codes file after each run as 
    # each run gives the same header key.
    print_routine("Extract NONSS spectra (spextract)")
    
    in_target_filename = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ".fits"
    out_target_filename = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ext_suffix + ".fits"

    output = Popen([extract, in_target_filename, "simple", "none", "4", "0", out_target_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATEX"):
                    f_new.write(line)
                else:
                    key         = line.split('\t')[0]
                    code        = line.split('\t')[1]   
                    desc        = line.split('\t')[2].strip('\n')       
                    f_new.write("L2STATX1\t" + code + "\t" + desc + " (target NONSS)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")     
    
    
    print_routine("Extract SS spectra (spextract)")
    
    in_target_filename = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ".fits"
    out_target_filename = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ext_suffix + ss_suffix + ".fits"

    output = Popen([extract, in_target_filename, "simple", "median", "4", "25", out_target_filename], stdout=PIPE)
    print output.stdout.read()  
    with open("new_error_codes", "w") as f_new:
        with open("error_codes") as f_old:
            for line in f_old:
                if not line.startswith("L2STATEX"):
                    f_new.write(line)
                else:
                    key         = line.split('\t')[0]
                    code        = line.split('\t')[1]   
                    desc        = line.split('\t')[2].strip('\n')       
                    f_new.write("L2STATX2\t" + code + "\t" + desc + " (target SS)\n")
    os.remove("error_codes")
    move("new_error_codes", "error_codes")        
    
    # --------------------
    # - RENAME DAT FILES -
    # --------------------   
    for i in os.listdir("."):
        if i.endswith(".dat"):
            if not i.startswith("p_"):
                filename = os.path.splitext(os.path.basename(i))[0]
                ext = os.path.splitext(os.path.basename(i))[1]
                move(i, "p_" + filename + "_target_corrected" + ext)   
                
    # ------------------------------
    # - REFORMAT FILE (SPREFORMAT) -
    # ------------------------------
    print_routine("Reformat spectra (spreformat)")
    
    in_target_headers_filename = target + target_suffix + ".fits"    
    in_target_filename_L1_IMAGE = target + target_suffix + ".fits"    
    in_target_filename_LSS_NONSS = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ".fits"
    in_target_filename_SPEC_NONSS = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ext_suffix + ".fits"
    in_target_filename_SPEC_SS = target + target_suffix + trim_suffix + cor_suffix + reb_suffix + ext_suffix + ss_suffix + ".fits"
    
    out_target_filename = target[:-1] + "2.fits"
    
    # L1_IMAGE
    output = Popen([reformat, in_target_filename_L1_IMAGE, in_target_headers_filename, "L1_IMAGE", out_target_filename], stdout=PIPE)
    print output.stdout.read()           
    
    # LSS_NONSS
    output = Popen([reformat, in_target_filename_LSS_NONSS, in_target_headers_filename, "LSS_NONSS", out_target_filename], stdout=PIPE)
    print output.stdout.read()   
    
    # SPEC_NONSS
    output = Popen([reformat, in_target_filename_SPEC_NONSS, in_target_headers_filename, "SPEC_NONSS", out_target_filename], stdout=PIPE)
    print output.stdout.read() 
    
    # SPEC_SS
    output = Popen([reformat, in_target_filename_SPEC_SS, in_target_headers_filename, "SPEC_SS", out_target_filename], stdout=PIPE)
    print output.stdout.read() 
    
    # ----------------------------------------------
    # - GENERATE RASTER PLOT OF L1_IMAGE extension -
    # ----------------------------------------------
    print_routine("Plot extensions of output file (l2pi)")     
    in_target_filename = target[:-1] + "2.fits"

    output = Popen(["python", plot_image, "--f", in_target_filename, "--hdu", "L1_IMAGE", "--o", target_output_L1_IMAGE, "--ot", "L1_IMAGE"], stdout=PIPE)
    print output.stdout.read()  
    output.wait()
    if os.path.exists(target_output_L1_IMAGE) and output.returncode == 0:
        print_notification("Success.")
    else:
        print_notification("Failed.")
        exit(1)
        
    # ------------------------------------------------
    # - GENERATE RASTER PLOT OF SPEC_NONSS extension -
    # ------------------------------------------------      
    print_routine("Plot extensions of output file (l2ps)")     
    in_target_filename = target[:-1] + "2.fits"    
        
    output = Popen(["python", plot_spec, "--f", in_target_filename, "--hdu", "SPEC_NONSS", "--o", target_output_SPEC_NONSS, "--ot", "SPEC_NONSS"], stdout=PIPE)
    print output.stdout.read()  
    output.wait()
    if os.path.exists(target_output_SPEC_NONSS) and output.returncode == 0:
        print_notification("Success.")
    else:
        print_notification("Failed.")
        exit(1)
        
    # ------------------------------------------------
    # - GENERATE RASTER PLOT OF SPEC_NONSS extension -
    # ------------------------------------------------      
    print_routine("Plot extensions of output file (l2ps)")     
    in_target_filename = target[:-1] + "2.fits"    
        
    output = Popen(["python", plot_spec, "--f", in_target_filename, "--hdu", "SPEC_SS", "--o", target_output_SPEC_SS, "--ot", "SPEC_SS"], stdout=PIPE)
    print output.stdout.read()  
    output.wait()
    if os.path.exists(target_output_SPEC_SS) and output.returncode == 0:
        print_notification("Success.")
    else:
        print_notification("Failed.")
        exit(1)        
	        
	
    

