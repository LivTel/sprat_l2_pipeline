#!/bin/bash
CURRENT_PATH=$(pwd)
DOCKERFILE_PATH=$(pwd)/docker

# clean temporary directory
if [ "$(docker ps -q -f name=$DOCKERFILE_PATH/output/)" ]; then
    rm $DOCKERFILE_PATH/output/*
fi

# If l2_pipeline docker image does not exist, build one
if [ "$(docker images -q l2_pipeline_image)" == "" ]; then
  docker build -f docker/Dockerfile.l2_pipeline -t l2_pipeline_image .
fi

# If sprat_l2_pipeline docker image does not exist, build one
if [ "$(docker images -q sprat_l2_pipeline_image)" == "" ]; then
  docker build -f docker/Dockerfile.sprat_l2_pipeline -t sprat_l2_pipeline_image .
fi

# If orphan container exist, stop and remove it
if [ "$(docker ps -q -f name=sprat_l2_pipeline_container)" ]; then
    echo "Stopping orphan container"
    docker stop sprat_l2_pipeline_container
    echo "Removing orphan container"
    docker rm sprat_l2_pipeline_container
fi

# If an argument "rebuild" is providied, remove and rebuild the docker image
if [[ ("$1" = "rebuild") || ("$2" = "rebuild") ]]; then
    docker rmi sprat_l2_pipeline_image
    docker build -f docker/Dockerfile.sprat_l2_pipeline -t sprat_l2_pipeline_image .
fi

# -id :: runs a detached container interactively
# --name :: provide a name to the container, or a random name will be given
# -v :: mounting a docker volume [path_on_the_host:path_in_the_container]
# last argument :: the docker image to be run by the container
docker run -id --name sprat_l2_pipeline_container \
    -v /data/incoming:/space/home/dev/src/output \
    -v /data/Dprt/sprat:/data/Dprt/sprat \
    -v /data/incoming:/data/incoming \
    -v /usr/loca/bin/sprat_l2_pipeline/config:/space/home/dev/src/sprat_l2_pipeline/config \
    sprat_l2_pipeline_image


# If an argument "start" is providied, start docker container without invoking the pipeline
if [[ ("$1" = "start") || ("$2" = "start") ]]; then
    echo "A detached sprat_l2_pipeline_container is running, enter the container by the following command:"
    echo ""
    echo "  docker exec -it sprat_l2_pipeline_container [sh/bash/tcsh]"
    echo ""
    echo "(The pipeline environment is decleared in tcsh script at sprat_l2_pipeline/scripts/L2_setup)"
    echo ""
    echo "To clean up, use the following commands:"
    echo ""
    echo "  docker stop sprat_l2_pipeline_container"
    echo ""
    echo ""
else
    # Run the pipeline as executable
    # This should be modified to take a list of file names
    #docker exec sprat_l2_pipeline_container tcsh -c \
    #    "sudo -u data tcsh -c 'l2exec \
    #    --t=/data/incoming/v_s_20190109_37_1_0_1.fits \
    #    --r=/data/Dprt/sprat/Reference/v_reference_red_1.fits \
    #    --c=/data/Dprt/sprat/Continuum/v_continuum_red_1.fits \
    #    --a=/data/Dprt/sprat/Arc/v_a_20190109_47_1_0_1.fits \
    #    --f=/data/Dprt/sprat/FluxCorrection/red_cal0_1.fits \
    #    --dir=output \
    #    --o'"
    echo ""
    # Clean up (takes a few seconds)
#    echo "Stopping container"
#    docker stop sprat_l2_pipeline_container
#    echo "Removing container"
#    docker rm sprat_l2_pipeline_container
fi
