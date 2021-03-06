import inspect
import time

class errors():
    def __init__(self):
        self.current_code = 0
        self.codes = []
        self.err_dict = {
            0   :       "SUCCESS.",
            1   :       "CRITICAL. Input files are undefined.",
            2   :       "CRITICAL. Target file doesn't exist.",
            3   :       "CRITICAL. Reference file doesn't exist.",
            4   :       "CRITICAL. Continuum file doesn't exist.",
            5   :       "CRITICAL. Arc file doesn't exist.",
            6   :       "WARNING. Couldn't make ref_pre_sdist_plot plot.",
            7   :       "WARNING. Couldn't make ref_post_sdist_plot plot.",
            8   :       "CRITICAL. Reference spectrum still has significant curvature.",
            9   :       "WARNING. Couldn't make L1_IMAGE plot.",
            10  :       "WARNING. Couldn't make SPEC_NONSS plot.",
            11  :       "WARNING. Couldn't make SPEC_SS plot.",
            12  :       "WARNING. Couldn't make montage plot.",
            13  :       "CRITICAL. Negative error codes exist in error_codes file.",
            14  :       "CRITICAL. Working directory not empty and clobber not set.",
            15  :       "CRITICAL. Unable to create working directory.",
            16  :       "CRITICAL. Failed to find GRATROT and/or DATE-OBS key.",
            17  :       "CRITICAL. Invalid type for GRATROT key.",
            18  :       "CRITICAL. Invalid value for GRATROT key.",
            19  :       "CRITICAL. Failed to find suitable arc reference file for given exposure datetime and binning.",
            20  :       "WARNING. spclip call gave non-zero return.",
            21  :       "WARNING. spfind call gave non-zero return.",
            22  :       "WARNING. sptrace call gave non-zero return.",     
            23  :       "WARNING. spcorrect call gave non-zero return.",
            24  :       "WARNING. sparcfit call gave non-zero return.",
            25  :       "WARNING. sprebin call gave non-zero return.", 
            26  :       "WARNING. spextract call gave non-zero return.",
            27  :       "WARNING. spreformat call gave non-zero return.",
            28  :       "CRITICAL. Couldn't find config file.",
            29  :       "CRITICAL. Failed to find suitable config file for given exposure datetime and binning.",
	    30  :       "WARNING. spflcor call gave non-zero return on either FLCOR or FLAMBDA.",
            31  :       "WARNING. ACQFAUTO or EXPTIME keywrds required for flux calibration not found.", 
            32  :       "WARNING. Couldn't make FLCOR_SS plot.",
            33  :       "CRITICAL. Flux correction file doesn't exist."
        }
        
    def set_code(self, code, is_fatal=True):
        self.current_code = code
        parent_calling_func = inspect.stack()[1][3]
        print time.strftime("%Y-%m-%d %H:%M:%S") + " - [" + parent_calling_func + "] - " + "Code:\t" + str(self.current_code)
        print time.strftime("%Y-%m-%d %H:%M:%S") + " - [" + parent_calling_func + "] - " + "Message:\t\"" + self.err_dict[self.current_code] + "\""
        print
        if is_fatal:
            exit(self.current_code)
            
    def get_code(self):
        return self.current_code
        
