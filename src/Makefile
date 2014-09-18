##################################
# Makefile 			 #
# Last Modified Date:   11/09/14 #
##################################

#############################
# compilers, archivers etc. #
#############################

CC		= /usr/bin/gcc
AR		= /usr/bin/ar

##################
# compiler flags #
##################

CFLAGS		= -Wall -g -std=c99

#########################
# csh terminal commands #
#########################

CAT		= /bin/cat
GREP		= /bin/grep
MV		= /bin/mv
RM		= /bin/rm -i

################
# $(LIBS) flag #
################

LIBS		= -L$(L2_LIB_DIR)#						rmb/laptop
#LIBS		= -L/space/home/dev/src/cfitsio3310 -L$(L2_LIB_DIR)#		lt/ltdevsrv

####################
# $(INCLUDES) flag #
####################

INCLUDES	= -I/usr/include/cfitsio -I/usr/include/gsl#			rmb/laptop
#INCLUDES	= -I/space/home/dev/src/cfitsio3310 -I/usr/include/gsl#		lt/ltdevsrv

################
# make default #
################

all: sprat_red_findedges

#################
# make routines #
#################

sprat_red_trim: sprat_red_trim.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_trim.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o sptrim
	$(MV) sptrim $(L2_BIN_DIR)

sprat_functions: sprat_functions.c
	$(CC) $(CFLAGS) $(INCLUDES) -c sprat_functions.c

sprat_error_handling: sprat_error_handling.c
	$(CC) $(CFLAGS) $(INCLUDES) -c sprat_error_handling.c

############
# libsprat #
############

libsprat: sprat_functions sprat_error_handling
	$(AR) rcs libsprat.a sprat_functions.o sprat_error_handling.o
	$(MV) libsprat.a $(L2_LIB_DIR)
	$(RM) -f *.o

##############
# make clean #
##############

clean:
	$(RM) $(L2_LIB_DIR)/*
	$(RM) $(L2_BIN_DIR)/*

