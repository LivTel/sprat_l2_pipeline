'''
  - Extracts the X,Y data from a FITS HDU ready for plotting 
'''

import matplotlib
from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np
from optparse import OptionParser

#PLOT_PADDING = 20

def execute(f_in, hdu): 
  hdulist = pyfits.open(f_in)
  data = hdulist[hdu].data
  hdrs = hdulist[hdu].header
  
  CRVAL1        = hdrs['CRVAL1']
  CDELT1        = hdrs['CDELT1']
  NAXIS1        = hdrs['NAXIS1']
  hdulist.close()
  
  x = []
  for i in range(0, NAXIS1, 1):
      x.append(CRVAL1 + i*CDELT1)
  
  y = data.reshape(NAXIS1)
        
  return (x,y)

if __name__ == "__main__":
  parser = OptionParser()
  parser.add_option('--f', dest='f_in', action='store', default='out.fits')
  parser.add_option('--hdu', dest='hdu', action='store')
  (options, args) = parser.parse_args()
  
  f_in		= str(options.f_in)
  hdu 		= str(options.hdu)
  
  execute(f_in, hdu)
  

                  
