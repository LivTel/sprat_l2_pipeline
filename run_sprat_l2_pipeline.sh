#!/bin/bash
CURRENT_PATH=$(pwd)

# clean temporary directory
if [ "$(docker ps -q -f name=$CURRENT_PATH/output_test/)" ]; then
    rm $CURRENT_PATH/output_test/*
fi

# If docker image does not exist, build one
if [ "$(docker images -q sprat_l2_pipeline_image)" == "" ]; then
  docker build -t sprat_l2_pipeline_image .
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
    docker build -t sprat_l2_pipeline_image .
fi

# -id :: runs a detached container interactively
# --name :: provide a name to the container, or a random name will be given
# -v :: mounting a docker volume [path_on_the_host:path_in_the_container]
# last argument :: the docker image to be run by the container
docker run -id --name sprat_l2_pipeline_container -v $CURRENT_PATH/output_test:/space/home/dev/src/output_test sprat_l2_pipeline_image

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
    echo "  docker rm sprat_l2_pipeline_container"
    echo ""
else
    # Run the pipeline as executable
    # This should be modified to take a list of file names
    docker exec sprat_l2_pipeline_container tcsh -c "python sprat_l2_pipeline/scripts/L2_exec.py --r=sprat/Reference/v_s_20180627_51_1_0_1.fits --c=sprat/Continuum/v_w_20141121_1_1_0_1.fits --a=sprat/Arc/v_a_20180627_54_1_0_1.fits --f=sprat/FluxCorrection/red_cal0_1.fits --dir=output_test --o"

    # Clean up (takes a few seconds)
    echo "Stopping container"
    docker stop sprat_l2_pipeline_container
    echo "Removing container"
    docker rm sprat_l2_pipeline_container
fi
