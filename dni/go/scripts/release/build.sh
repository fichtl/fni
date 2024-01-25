#!/bin/bash

set -e

OS="linux"  # only supportted
ARCH="amd64"

pushd .  1>/dev/null  
cd ../../
source  path_env.sh
popd    1>/dev/null  

 
PACKET_PATH=${RELEASE_DIR}/dni/

mkdir -p ${PACKET_PATH}

pushd . 1>/dev/null

GOLDFLAG="-s -w"
# cd ${SNDCTL_DIR}
# GOOS=${OS} GOARCH=${ARCH} go build  -ldflags "${GOLDFLAG}"  *.go  
# sudo mv sndctl ${RELEASE_DIR}/snds/

cd ${DNI_DIR}
GOOS=${OS} GOARCH=${ARCH} go build  -ldflags "${GOLDFLAG}"  dni.go option.go  
sudo mv dni ${RELEASE_DIR}/dni/

popd  1>/dev/null

# sudo cp -f ${CONFIG_DIR}/snds.conf ${RELEASE_DIR}/snds/
 
sudo cp -f ${SCRIPT_DIR}/service/start_service.sh  ${RELEASE_DIR}/dni/
sudo cp -f ${SCRIPT_DIR}/service/dni.service.in  ${RELEASE_DIR}/dni/

sudo tar -zcvf ${RELEASE_DIR}/dni.tar.gz  -C ${RELEASE_DIR}/ dni  1>/dev/null 
rm -rf ${RELEASE_DIR}/dni