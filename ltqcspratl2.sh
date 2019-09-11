sprat_l2_pipeline_path='/usr/local/bin/sprat_l2_pipeline/docker'
CURRENT_DIR=$(pwd)

# ---------------------------------- #
# List docker images (type)          #
# (type) : all, l2_pipeline or sprat #
# ---------------------------------- #

list_l2_pipeline_image() {
    case ${1:-all} in
        all)
            docker images "*l2_pipeline_image*"
        ;;
        l2_pipeline)
            docker images "*l2_pipeline_image*"
        ;;
        sprat)
            docker images "*sprat_l2_pipeline_image*"
        ;;
        *)
            echo 'Allowed options are all, l2_pipeline and sprat. Default is all.'
        ;;
    esac
}


# ------------------------------------------------------------------------------------------------------- #
# Build l2_pipeline images (type)                                                                         #
# This is the base image that contain all the external build tools and libraries, i.e. the numerical part #
# (action) : build or rebuild                                                                             #
# ------------------------------------------------------------------------------------------------------- #

build_l2_pipeline_image(){
    case ${1:-nothing} in
        build)
            if [ ! "$(docker images -q l2_pipeline_image)" == "" ]; then
                echo 'sprat_l2_pipeline_image already exist. Try rebuild or update.'
                break
            else
                docker build -f $sprat_l2_pipeline_path/Dockerfile.l2_pipeline -t l2_pipeline_image .
            fi
        ;;
        rebuild)
            if [ "$(docker images -q l2_pipeline_image)" == "" ]; then
                echo 'sprat_l2_pipeline_image does not exist. Building one instead.'
                docker build -f $sprat_l2_pipeline_path/Dockerfile.l2_pipeline -t l2_pipeline_image .
            else
                echo 'l2_pipeline_image is rebuilding from scratch.'
                docker build --no-cache=true -f $sprat_l2_pipeline_path/Dockerfile.l2_pipeline -t l2_pipeline_image .
            fi
        ;;
        *)
            echo 'Please choosing from build or rebuild.'
        ;;
    esac
}


# --------------------------------------------------------- #
# Build sprat_l2_pipeline images (type)                     #
# This is the logical part of the sprat l2 pipeline image   #
# (action) : build or rebuild                               #
# --------------------------------------------------------- #
build_sprat_l2_pipeline_image(){
    case ${1:-nothing} in
        build)
            if [ ! "$(docker images -q sprat_l2_pipeline_image)" == "" ]; then
                echo 'sprat_l2_pipeline_image already exist. Try rebuild or update.'
                break
            else
                docker build -f $sprat_l2_pipeline_path/Dockerfile.sprat_l2_pipeline -t sprat_l2_pipeline_image .
            fi
        ;;
        rebuild)
            if [ "$(docker images -q sprat_l2_pipeline_image)" == "" ]; then
                echo 'sprat_l2_pipeline_image does not exist. Building one instead of rebuilding.'
                docker build -f $sprat_l2_pipeline_path/Dockerfile.sprat_l2_pipeline -t sprat_l2_pipeline_image .
            else
                echo 'sprat_l2_pipeline_image is rebuilding from scratch.'
                docker build --no-cache=true -f $sprat_l2_pipeline_path/Dockerfile.sprat_l2_pipeline -t sprat_l2_pipeline_image .
            fi
        ;;
        *)
            echo 'Please choosing from build or rebuild.'
        ;;
    esac
}


# ------------------------------------------------------------ #
# List all spratl2(type) container IDs with state (status)     #
# (type) : all, daily, quicklook, instance_nnn or all_instance #
# (status): all, running, exited, dead                         #
# ------------------------------------------------------------ #

list_spratl2_container_id() {
    case ${1:-all} in
        all)
            case ${2:-all} in
                all)
                    docker ps -a -f name="sprat_l2_pipeline_*" -q
                    ;;
                running)
                    docker ps -a -f status=running -f name="sprat_l2_pipeline_*" -q
                    ;;
                exited)
                    docker ps -a -f status=exited -f name="sprat_l2_pipeline_*" -q
                    ;;
                dead)
                    docker ps -a -f status=dead -f name="sprat_l2_pipeline_*" -q
                    ;;
                *)
                    echo 'Allowed sprat l2 container status are: all (default if leave blank), running, exited or dead'
                    ;;
            esac
        ;;
        daily|quicklook|instance_[0123456789][0123456789][0123456789])
            case ${2:-all} in
                all)
                    docker ps -a -f name="sprat_l2_pipeline_$1" -q
                    ;;
                running)
                    docker ps -a -f status=running -f name="sprat_l2_pipeline_$1" -q
                    ;;
                exited)
                    docker ps -a -f status=exited -f name="sprat_l2_pipeline_$1" -q
                    ;;
                dead)
                    docker ps -a -f status=dead -f name="sprat_l2_pipeline_$1" -q
                    ;;
                *)
                    echo 'Allowed sprat l2 container status are: all (default if leave blank), running, exited or dead'
                    ;;
            esac
        ;;
        all_instance)
            case ${2:-all} in
                all)
                    docker ps -a -f name="sprat_l2_pipeline_instance*" -q
                    ;;
                running)
                    docker ps -a -f status=running -f name="sprat_l2_pipeline_instance*" -q
                    ;;
                exited)
                    docker ps -a -f status=exited -f name="sprat_l2_pipeline_instance*" -q
                    ;;
                dead)
                    docker ps -a -f status=dead -f name="sprat_l2_pipeline_instance*" -q
                    ;;
                *)
                    echo 'Allowed sprat l2 container status are: all, running, exited or dead'
                    ;;
            esac
        ;;
        *)
            echo 'Allowed sprat l2 container types are: all (default if leave blank), daily, quicklook, instance_nnn or all_instance'
        ;;
    esac
}


# ----------------------------------------------------------- #
# Show all spratl2(type) container with state (status)        #
# (type) : all, daily, quicklook, instance_nn or all_instance #
# (status): all, running, exited, dead                        #
# ----------------------------------------------------------- #

show_spratl2_container() {
    containers="$(list_spratl2_container_id $1)"
    counter=0
    if [[ $containers = '' ]]; then
        echo 'Cannot find container: '$1'.'
    fi
    for i in $containers; do
        {
            if [[ ! counter -gt 0 ]]; then
                docker ps -a -f id=$i
            else
                docker ps -a -f id=$i | awk 'NR==2'
            fi
        }
        counter=$((counter+1))
    done
}


# ----------------------------------- #
# Create new spratl2(type) container  #
# (type) : daily, quicklook, instance #
# default is to create an instance.   #
# ----------------------------------- #
# as of 19 Aug 2019
# daily and quicklook containers have deafult folder path; but instance **requires** folder path or else throw errors 
# default DAILY
#            docker run -id --name sprat_l2_pipeline_daily \
#                -v /data/incoming:/space/home/dev/src/output \
#                -v /data/Dprt/sprat:/data/Dprt/sprat \
#                -v /data/incoming:/data/incoming \
#                -v /usr/local/bin/sprat_l2_pipeline/config:/space/home/dev/src/sprat_l2_pipeline/config \
#                sprat_l2_pipeline_image
# default QUICKLOOK
#            docker run -id --name sprat_l2_pipeline_quicklook \
#                -v /data/incoming:/space/home/dev/src/output \
#                -v /data/Dprt/sprat:/data/Dprt/sprat \
#                -v /data/incoming:/data/incoming \
#                -v /usr/local/bin/sprat_l2_pipeline/config:/space/home/dev/src/sprat_l2_pipeline/config \
#                sprat_l2_pipeline_image

create_spratl2_container() {
    if [ ! -d '$2' ]; then
        echo 'Argument 2: '$2' is NOT a valid folder path.'
        echo 'Default path is used: '$CURRENT_DIR
    fi
    if [ ! -d '$3' ]; then
        echo 'Argument 3: '$3' is NOT a valid folder path.'
        echo 'Default path is used: /data/Dprt/sprat'
    fi
    if [ ! -d '$4' ]; then
        echo 'Argument 4: '$4' is NOT a valid folder path.'
        echo 'Default path is used: '$CURRENT_DIR
    fi
    if [ ! -d '$5' ]; then
        echo 'Argument 5: '$5' is NOT a valid folder path.'
        echo 'Default path is used: /usr/local/bin/sprat_l2_pipeline/config'
    fi
    case ${1:-instance} in
        daily)
            docker run -id --name sprat_l2_pipeline_daily \
                -v ${2:-/data/incoming}:/space/home/dev/src/output \
                -v ${3:-/data/Dprt/sprat}:/data/Dprt/sprat \
                -v ${4:-/data/incoming}:/data/incoming \
                -v ${5:-/usr/local/bin/sprat_l2_pipeline/config}:/space/home/dev/src/sprat_l2_pipeline/config \
                sprat_l2_pipeline_image
        ;;
        quicklook)
            docker run -id --name sprat_l2_pipeline_quicklook \
                -v ${2:-/home/data/QL/Reduction}:/space/home/dev/src/output \
                -v ${3:-/data/Dprt/sprat}:/data/Dprt/sprat \
                -v ${4:-/home/data/QL/Reduction}:/data/incoming \
                -v ${5:-/usr/local/bin/sprat_l2_pipeline/config}:/space/home/dev/src/sprat_l2_pipeline/config \
                sprat_l2_pipeline_image
        ;;
        instance)
            nnn=$(echo $(($(docker ps -a -f name="sprat_l2_pipeline_instance_*" -q | wc -l ) + 1001 )) | cut -c2-)
            docker run -id --name sprat_l2_pipeline_instance_$nnn \
                -v ${2:-$CURRENT_DIR}:/space/home/dev/src/output \
                -v ${3:-/data/Dprt/sprat}:/data/Dprt/sprat \
                -v ${4:-$CURRENT_DIR}:/data/incoming \
                -v ${5:-/usr/local/bin/sprat_l2_pipeline/config}:/space/home/dev/src/sprat_l2_pipeline/config \
                sprat_l2_pipeline_image
        ;;
        *)
            echo 'Allowed sprat l2 container types are: daily, quicklook, instance or [leave blank].'
        ;;
    esac
}


# -------------------------------------------------------- #
# Remove spratl2(type) container wiht a known container id #
# -------------------------------------------------------- #

remove_spratl2_container_by_id() {
    docker stop $1 | xargs docker rm
}


# ------------------------------------------------------------ #
#                       *** DANGER ***                         #
#                      USE WITH CAUTION                        #
# ------------------------------------------------------------ #
# Remove spratl2(type) container by type                       #
# (type) : all, daily, quicklook, instance_nnn or all_instance #
# ------------------------------------------------------------ #

remove_spratl2_container_by_type() {
    case $1 in
        all|daily|quicklook|all_instance)
            while true; do
                read -p "Are you sure you wish to REMOVE $1 spratl2 container(s)?" yn
                case $yn in
                    YES|Yes|yes|Y|y)
                        containers="$(list_spratl2_container_id $1)"
                        if [[ $containers = '' ]]; then
                            if [[ $1 = 'all' ]]; then
                                echo 'Cannot find any spratl2 container.'
                            else
                                echo 'Cannot find container of type: '$1'.'
                            fi
                            break
                        fi
                        for i in $containers; do
                            echo 'Removed '$(remove_spratl2_container_by_id $i)
                        done
                        break
                    ;;
                    NO|No|no|N|n)
                        echo 'Nothing is removed.'
                        break
                    ;;
                esac
            done
        ;;
        instance_[0123456789][0123456789][0123456789])
            containers="$(list_spratl2_container_id $1)"
            if [[ $containers = '' ]]; then
                echo 'Cannot find container: sprat_l2_pipeline_'$1'.'
            fi
            for i in $containers; do
                echo 'Removed '$(remove_spratl2_container_by_id $i)
            done
        ;;
        *)
            echo 'Allowed sprat l2 container types are: all, daily, quicklook, instance_nnn or all_instance'
        ;;
    esac
}



