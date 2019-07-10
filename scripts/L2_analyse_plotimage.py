'''
  - Produces a plot of the L1 image.
'''

import sys
from optparse import OptionParser

from astropy.io import fits
import numpy as np
import matplotlib
matplotlib.use('Agg')

from matplotlib import pyplot as plt

PLOT_PADDING = 20


def execute(f_in, hdu, f_out, plt_title, save=True, hold=False):
    hdulist = fits.open(f_in)
    data = hdulist[hdu].data

    plt.imshow(
        data,
        aspect='auto',
        vmin=np.median(data),
        vmax=np.percentile(data, 99.5),
        cmap="Greys")
    plt.xlabel("x")
    plt.ylabel("y")

    if save:
        plt.savefig(f_out)
    if not hold:
        plt.clf()

    return 0


if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option('--f', dest='f_in', action='store', default='out.fits')
    parser.add_option('--hdu', dest='hdu', action='store')
    parser.add_option('--o', dest='f_out', action='store', default='plot.png')
    parser.add_option('--ot', dest='plt_title', action='store', default='plot')
    (options, args) = parser.parse_args()

    f_in = str(options.f_in)
    hdu = str(options.hdu)
    f_out = str(options.f_out)
    plt_title = str(options.plt_title)

    execute(f_in, hdu, f_out, plt_title)