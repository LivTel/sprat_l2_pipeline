import sys
from binit import bin_image

"""
  Script to create a set of binned images for testing.
 
  Last modified:
  3 February 2019

  Maintainer:
  Marco C Lam (@cylammarco)
 
  Organisation:
  (1) Liverpool Telescope, ARI, LJMU
  
  Contact:
  C.Y.Lam@ljmu.ac.uk
"""

if __name__ == "__main__":

    f_name = sys.argv[1]
    bin_x = int(sys.argv[2])
    bin_y = int(sys.argv[3])
    f_out = f_name.split('.')[0].split('/')[-1] + '_binned_' + str(bin_x) + '_' + str(bin_y) + '.fits'

    print(bin_image(f_name, f_out, bin_x, bin_y))
