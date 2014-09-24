from matplotlib import pyplot as plt
import pyfits
import sys
import numpy as np

dat = sys.argv[1]
hdulist = pyfits.open(sys.argv[2])
data = hdulist[0].data

plt.imshow(data, aspect='auto', vmin=np.min(data), vmax=800)
plt.ylim([60,170])

x = []
y = []
with open(dat) as f:
  for line in f:
    if not line.startswith('#') and line.strip() != "" and not line.startswith('-1'):
      x.append(float(line.split('\t')[0]))
      y.append(float(line.split('\t')[1]))

fit_coeffs = np.polyfit(x, y, 2)
y_fitted = np.polyval(fit_coeffs, x)

print fit_coeffs

plt.plot(x, y, 'ko')
plt.plot(x, y_fitted, 'r-')
plt.colorbar()

plt.show()
