from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np
from optparse import OptionParser

PLOT_PADDING = 20

if __name__ == "__main__":
  parser = OptionParser()
  parser.add_option('--f', dest='f_in', action='store', default='out.fits')
  parser.add_option('--p', dest='dat_peaks', action='store', default='spfind_peaks.dat')
  parser.add_option('--t', dest='dat_traces', action='store', default='sptrace_traces.dat')
  parser.add_option('--o', dest='f_out', action='store', default='plot.png')
  parser.add_option('--ot', dest='plt_title', action='store', default='plot')
  (options, args) = parser.parse_args()
  
  f_in		= options.f_in
  dat_peaks  	= options.dat_peaks
  dat_traces 	= options.dat_traces
  f_out		= options.f_out
  plt_title	= options.plt_title
  
  hdulist = pyfits.open(f_in)
  data = hdulist[0].data
  
  plt.imshow(data, aspect='auto', vmin=np.median(data), vmax=np.percentile(data, 99.5))
  
  x = []
  y = []
  with open(dat_peaks) as f:
    for line in f:
      if not line.startswith('#') and line.strip() != "" and not line.startswith('-1'):
        x.append(float(line.split('\t')[0]))
        y.append(float(line.split('\t')[1]))
        
        coeffs = []
        with open(dat_traces) as f:
          for line in f:
            if line.startswith('# Polynomial Order'):
              ncoeffs = int(line.split('\t')[1]) + 1  
            elif not line.startswith('#') and line.strip() != "" and not line.startswith('-1'):
              for i in range(ncoeffs):
                coeffs.append(float(line.split('\t')[i]))
                  
  x_fitted = range(0, len(data[0]))
  y_fitted = np.polyval(coeffs[::-1], x_fitted)
                  
  y_min = min(y_fitted) - PLOT_PADDING
  y_max = max(y_fitted) + PLOT_PADDING
                  
  plt.plot(x, y, 'ko')
  plt.plot(x_fitted, y_fitted, 'r-')
  plt.colorbar()
  plt.ylim([y_min, y_max])
  plt.title(plt_title)
  plt.xlabel("x")
  plt.ylabel("y")
  plt.savefig(f_out)
                  