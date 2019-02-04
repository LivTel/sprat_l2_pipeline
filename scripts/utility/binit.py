import sys
import itertools
import warnings
import numpy as np
from astropy.io import fits


"""
  Script to bin image in either or both spatial and spectral direction
 
  Last modified:
  31 January 2019

  Maintainer:
  Marco C Lam (@cylammarco)
 
  Organisation:
  (1) Liverpool Telescope, ARI, LJMU
  
  Contact:
  C.Y.Lam@ljmu.ac.uk
"""

def bin_image(f_name, output_name, bin_x, bin_y):

    """
    Parameters
    ----------
    f_name : str
        File name
    output_name : str
        Output file path
    bin_x : int
        Binning in the x-direction
    bin_y : int
        Binning in the y-direction

    Returns
    -------
    error_code : int
        0 or 1
    """

    # Check bin_x is value and int
    if not isinstance(bin_x, (int)):
        bin_x = int(bin_x)
        warnings.warn('bin_x is a ' + str(type(bin_x)) + '. It is converted' +
            ' to int.')

    # Check bin_y is value and int
    if not isinstance(bin_y, (int)):
        bin_y = int(bin_y)
        warnings.warn('bin_y is a ' + str(type(bin_y)) + '. It is converted' +
            ' to int.')

    # Open file which contains only 1 HDU with 1 header and 1 image data
    f = fits.open(f_name)
    f_header = f[0].header
    f_data = f[0].data

    # Integer divide to find the number of pixels after binning
    f_y, f_x = np.shape(f_data)
    a_x = f_x // bin_x
    a_y = f_y // bin_y

    # New data array
    a_new = np.zeros((a_x, a_y))

    # Find the number 
    remain_x = f_x % bin_x
    remain_y = f_y % bin_y

    # Copy edge values to increase the array size in order to perform binning
    if remain_x > 0:
        for i in range(remain_x):
            f_data = np.insert(f_data, f_data.shape[0], values=f_data[-1], axis=0)
    if remain_y > 0:
        for i in range(remain_y):
            f_data = np.insert(f_data, f_data.shape[1], values=f_data[:,-1], axis=1)

    # Looping through the wavelength direction
    for a_pix_x, a_pix_y in list(itertools.product(np.arange(a_x), np.arange(a_y))):
        f_pix_x_1 = a_pix_x * bin_x
        f_pix_x_2 = (a_pix_x + 1) * bin_x
        f_pix_y_1 = a_pix_y * bin_y
        f_pix_y_2 = (a_pix_y + 1) * bin_y
        a_new[a_pix_x][a_pix_y] = np.sum(f_data[f_pix_y_1:f_pix_y_2][0][f_pix_x_1:f_pix_x_2])

    # Add new header keys
    a_new_header = f_header.copy()
    a_new_header['CCDXBIN'] = bin_x
    a_new_header['CCDYBIN'] = bin_y

    hdu = fits.PrimaryHDU(np.asarray(a_new.T), header=a_new_header)
    hdu.writeto(output_name)

    f.close()

    return 0
