# Dockerised sprat_l2_pipeline
=================

## This is a dockerised SPRAT L2 Pipeline that is run as an executable.

The Docker contains a centos:7.5.1804, the docker configurations are set at Dockerfile. It is installed with

* gcc, gcc-c++, kernel-devel, wget, git, curl, make, tcsh
* python-devel, libxslt-devel, libffi-devel, openssl-devel
* gsl (downloaded from http://mirror.switch.ch/ftp/mirror/gnu/gsl/gsl-2.5.tar.gz and compiled inside the docker)
* cfitsio (downloaded from https://src.fedoraproject.org/lookaside/pkgs/cfitsio/cfitsio3310.tar.gz/75b6411751c7f308d45b281b7beb92d6/cfitsio3310.tar.gz and compiled inside the docker)
* pip
* tkinter and python27-tkinter
* backports.functools-lru-cache, numpy, pyfits, matplotlib
* SPRAT L2 pipeline (comiled inside the docker)

The docker is run as an excutable by calling tcsh script to run the pipeline with ENTRYPOINT.
