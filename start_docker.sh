#!/bin/bash
CURRENT_PATH=$(pwd)

rm $CURRENT_PATH/output_test/*

# Build the docker image
docker build -t sprat_l2_pipeline_image .

# If the container sprat_l2_pipeline_container already exists, start it
# else, create one
#if [ "$(docker ps -q -f name=sprat_l2_pipeline_container)" ]; then
  #docker exec -it sprat_l2_pipeline_container tcsh
#else
  #docker run --name sprat_l2_pipeline_container -v $CURRENT_PATH/output_test:/space/home/dev/src/output_test sprat_l2_pipeline_image
  #docker exec -it sprat_l2_pipeline_container tcsh
#fi

docker run --name sprat_l2_pipeline_container -v $CURRENT_PATH/output_test:/space/home/dev/src/output_test sprat_l2_pipeline_image

# Clean up
docker rm sprat_l2_pipeline_container
