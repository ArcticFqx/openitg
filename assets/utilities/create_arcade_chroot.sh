#!/bin/sh

DEBIAN_SARGE_MIRROR="http://archive.debian.org/debian"
SARGE_DIST_NAME="sarge"

usage() {
	echo "Usage: $0 <path to source code> <chroot location>"
	exit
}

# used to check for needed files/programs
verify_prog_requirements() {
	# debootstrap is needed
	if [ "x`which debootstrap`" = "x" ]; then
		echo "$0: cannot continue, please install debootstrap on your system"
		exit
	fi
	echo "debootstrap location: `which debootstrap`"

	# user must be root
	if [ ! "x`whoami`" = "xroot" ]; then
		echo "$0: you must be root to setup the chroot environment"
		exit
	fi
}


# sets up debian sarge system at the given path, binds src dir to directory within chroot
setup_chroot() {
	CHROOT_DIR="$1"
	SRC_DIR="$2"
	echo "Setting up chroot at $CHROOT_DIR..."

	mkdir -p $CHROOT_DIR
	debootstrap $SARGE_DIST_NAME $CHROOT_DIR $DEBIAN_SARGE_MIRROR/
	if [ $? != 0 ]; then
		echo "$0: debootstrap failed, exiting"
		exit
	fi

	mkdir $CHROOT_DIR/root/openitg-dev


	# first time setup script (sets up debian, retrieves correct packages)
	cat <<! >$CHROOT_DIR/root/first_time_setup.sh
#!/bin/sh
export LC_ALL=C
echo -ne "*******************\n* You are now in the OpenITG AC chroot environment\n*******************\n"
echo "Installing necessary dev packages..."
# non-free and contrib for the nvidia packages
echo "deb $DEBIAN_SARGE_MIRROR $SARGE_DIST_NAME main contrib non-free" >/etc/apt/sources.list
apt-get update
apt-get install build-essential gettext automake1.8 gcc g++ libavcodec-dev libavformat-dev libxt-dev libogg-dev libpng-dev libjpeg-dev libvorbis-dev libusb-dev libglu1-mesa-dev libx11-dev libxrandr-dev liblua50-dev liblualib50-dev nvidia-glx-dev libmad0-dev libasound-dev
echo "OpenITG AC chroot successfully set up!"
exec /bin/bash
!
	chmod +x $CHROOT_DIR/root/first_time_setup.sh

	# bootstrap script
	cat <<! >$CHROOT_DIR/chroot_bootstrap.sh
#!/bin/sh
mount -o bind /proc $CHROOT_DIR/proc
mount -o bind /sys $CHROOT_DIR/sys
mount -o bind $SRC_DIR $CHROOT_DIR/root/openitg-dev
chroot $CHROOT_DIR /bin/bash
!
	chmod +x $CHROOT_DIR/chroot_bootstrap.sh

	mount -o bind /proc $CHROOT_DIR/proc
	mount -o bind /sys $CHROOT_DIR/sys
	mount -o bind $SRC_DIR $CHROOT_DIR/root/openitg-dev
	chroot $CHROOT_DIR /root/first_time_setup.sh
}

if [ "x$1" = "x" ]; then
	usage
fi
if [ "x$2" = "x" ]; then
	usage
fi

if [ ! -d "$1" ]; then
	echo "$0: Source code dir not found at $1"
	exit
fi

verify_prog_requirements

SRC_DIR="`(cd $1; pwd)`"
setup_chroot $2 $SRC_DIR

