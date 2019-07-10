# Dockerised sprat_l2_pipeline

The Docker contains a centos:7.5.1804, the docker configurations are set at Dockerfile. It is installed with

* gcc, gcc-c++, kernel-devel, wget, git, curl, make, tcsh
* python-devel, libxslt-devel, libffi-devel, openssl-devel
* gsl (downloaded from http://mirror.switch.ch/ftp/mirror/gnu/gsl/gsl-2.5.tar.gz and compiled inside the docker)
* cfitsio (downloaded from https://src.fedoraproject.org/lookaside/pkgs/cfitsio/cfitsio3310.tar.gz/75b6411751c7f308d45b281b7beb92d6/cfitsio3310.tar.gz and compiled inside the docker)
* pip
* tkinter and python27-tkinter
* backports.functools-lru-cache, numpy, pyfits, matplotlib
* SPRAT L2 pipeline (comiled inside the docker)

The shell script builds and starts the sprat_l2_pipeline docker image (sprat_l2_pipeline_image) and run it detached interactively (`-id`see definitions at https://docs.docker.com/engine/reference/commandline/run/#options). The container (sprat_l2_pipeline_container) mounts the docker volume to share input and output paths between the container and the host (`-v` flag). The pipeline is executed through `docker exec sprat_l2_pipeline_container tcsh -c "python (...)"`. After all the data reduction, the docker container is stopped and removed while the docker image stays on disk.

# How to use it

You may need to configure the shared paths from `Docker -> Preferences -> File Sharing` before the docker volume mounting works.

To run the pipeline, run `./run_sprat_l2_pipeline.sh`.

To start the container without invoking the pipeline, run `./run_sprat_l2_pipeline.sh start`. To enter the container, run `docker exec -it sprat_l2_pipeline_container [sh/bash/tcsh]`. To clean up afterwards, `docker stop sprat_l2_pipeline_container` and `docker rm sprat_l2_pipeline_container`. 

To rebuild the image from scratch before running the pipeline, run `./run_sprat_l2_pipeline.sh rebuild`.

To rebuild the image and start the container without invoking the pipeline, run `./run_sprat_l2_pipeline.sh rebuild start`.