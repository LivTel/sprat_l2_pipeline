'''
  - Produces a plot of the L1 image.
'''

from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np
from optparse import OptionParser

PLOT_PADDING = 20

if __name__ == "__main__":
  parser = OptionParser()
  parser.add_option('--f', dest='f_in', action='store', default='out.fits')
  parser.add_option('--hdu', dest='hdu', action='store')  
  parser.add_option('--o', dest='f_out', action='store', default='plot.png')
  parser.add_option('--ot', dest='plt_title', action='store', default='plot')
  (options, args) = parser.parse_args()
  
  f_in		= str(options.f_in)
  hdu 		= str(options.hdu)  
  f_out		= str(options.f_out)
  plt_title	= str(options.plt_title)
  
  hdulist = pyfits.open(f_in)
  data = hdulist[hdu].data
  
  plt.imshow(data, aspect='auto', vmin=np.median(data), vmax=np.percentile(data, 99.5))      
  plt.xlabel("x")
  plt.ylabel("y")
  plt.savefig(f_out)
  
  exit(0)
                  