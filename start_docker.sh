#!/bin/bash

# Build the docker image
docker build -t sprat_l2_pipeline_image .

# If the container sprat_l2_pipeline_container already exists, start it
# else, create one
if [ "$(docker ps -q -f name=sprat_l2_pipeline_container)" ]; then
  docker exec -it sprat_l2_pipeline_container tcsh
else
  docker run --name sprat_l2_pipeline_container -id sprat_l2_pipeline_image
  docker exec -it sprat_l2_pipeline_container tcsh
fi
