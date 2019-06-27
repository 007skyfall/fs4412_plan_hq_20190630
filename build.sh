#!/bin/sh

sec_path="CodeSign4SecureBoot/"
CPU_JOB_NUM=$(grep processor /proc/cpuinfo | awk '{field=$NF};END{print field+1}')
ROOT_DIR=$(pwd)
CUR_DIR=${ROOT_DIR##*/}

case "$1" in
	clean)
		echo make clean
		make mrproper
		;;
	*)
			
		if [ ! -d $sec_path ]
		then
			echo "**********************************************"
			echo "[ERR]please get the CodeSign4SecureBoot first"
			echo "**********************************************"
			return
		fi
		
		make fs4412_config
		
		make -j$CPU_JOB_NUM
		
		if [ ! -f checksum_bl2_14k.bin ]
		then
			echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
			echo "There are some error(s) while building uboot, please use command make to check."
			echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
			exit 0
		fi
		
		cp -rf checksum_bl2_14k.bin $sec_path
		cp -rf u-boot.bin $sec_path
		rm checksum_bl2_14k.bin
		
		cd $sec_path
		cat E4412_N.bl1.SCP2G.bin bl2.bin all00_padding.bin u-boot.bin tzsw_SMDK4412_SCP_2GB.bin > u-boot-fs4412.bin
		mv u-boot-fs4412.bin $ROOT_DIR
		
		rm checksum_bl2_14k.bin
		rm u-boot.bin

		echo 
		echo 
		;;
		
esac
