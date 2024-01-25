#!/bin/bash
pushd .  1>/dev/null  
cd ../../
source  path_env.sh
popd    1>/dev/null  

set -e

if [ "$1" != "deb" ] && [ "$1" != "rpm" ] && [ "$1" != "all" ] ; then
    echo "Error: please input package type {deb or rpm or all}"
    exit 1
fi


VERSION=$2 
sudo rm -rf ${RELEASE_DIR}
mkdir -p  ${RELEASE_DIR} 
bash build.sh


if [ "$1" == "deb" ] || [ "$1" == "all" ] ; then
    echo "Info: start to build debian package"
    tmp_dir=${RELEASE_DIR}/tmp/
    install_dir=/usr/local/ 
 
    mkdir -p ${tmp_dir}/${install_dir}

    cp -f ${RELEASE_DIR}/dni.tar.gz ${tmp_dir}/${install_dir}/
    cp -rf ./DEBIAN ${tmp_dir}/

    dpkg-deb -b ${tmp_dir} ${RELEASE_DIR}/dni.deb #&>/dev/null

    tarball=dni_deb${VERSION}
    mkdir -p ${RELEASE_DIR}/${tarball}
    cp ./install.sh ${RELEASE_DIR}/${tarball}
    cp ${RELEASE_DIR}/dni.deb      ${RELEASE_DIR}/${tarball}
    tar -zcvf ${RELEASE_DIR}/${tarball}.tar.gz  -C ${RELEASE_DIR}/ ${tarball}  1>/dev/null 
    sudo rm -rf ${RELEASE_DIR}/${tarball}
    sudo rm -rf ${tmp_dir}
    # cp ${RELEASE_DIR}/${tarball}.tar.gz ${SCRIPT_DIR}/docker/
fi

#rm -rf ${RELEASE_DIR}/dcube.tar.gz
echo "Info: succeed to build package"