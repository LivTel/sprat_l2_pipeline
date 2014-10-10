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

all: sprat_red_clip sprat_red_find sprat_red_trace_sdist sprat_red_correct_sdist sprat_red_arcfit sprat_red_extract

#################
# make routines #
#################

sprat_red_clip: sprat_red_clip.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_clip.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o spclip
	$(MV) spclip $(L2_BIN_DIR)
	
sprat_red_find: sprat_red_find.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_find.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o spfind
	$(MV) spfind $(L2_BIN_DIR)
	
sprat_red_trace_sdist: sprat_red_trace_sdist.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_trace_sdist.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o sptrace
	$(MV) sptrace $(L2_BIN_DIR)	
	
sprat_red_correct_sdist: sprat_red_correct_sdist.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_correct_sdist.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o spcorrect
	$(MV) spcorrect $(L2_BIN_DIR)	
	
sprat_red_arcfit: sprat_red_arcfit.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_arcfit.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o sparcfit
	$(MV) sparcfit $(L2_BIN_DIR)		
	
sprat_red_extract: sprat_red_extract.c libsprat
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) sprat_red_extract.c -lsprat -lgsl -lgslcblas -lcfitsio -lm -o spextract
	$(MV) spextract $(L2_BIN_DIR)		

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

