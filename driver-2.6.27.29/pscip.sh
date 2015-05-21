#!/bin/bash

KERNEL=`uname -r`
DRIVER_DIR=$SICILIA/drivers/$KERNEL/pscip

module="pscip"
device="pscip"
mode="666"

/sbin/insmod "$DRIVER_DIR/$module.ko" pscip_dev_number=16  || exit 1

# retrieve major number
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

# Remove stale nodes and replace them, then give gid and perms
rm -f /dev/${device}[0-9] /dev/${device}1[0-5]
mknod /dev/${device}0 c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3
mknod /dev/${device}4 c $major 4
mknod /dev/${device}5 c $major 5
mknod /dev/${device}6 c $major 6
mknod /dev/${device}7 c $major 7
mknod /dev/${device}8 c $major 8
mknod /dev/${device}9 c $major 9
mknod /dev/${device}10 c $major 10
mknod /dev/${device}11 c $major 11
mknod /dev/${device}12 c $major 12
mknod /dev/${device}13 c $major 13
mknod /dev/${device}14 c $major 14
mknod /dev/${device}15 c $major 15
chown -v sicilia.Control /dev/${device}* 
ln -sf ${device}0 /dev/${device}

chown -v sicilia.Control /dev/${device}[0-9] /dev/${device}1[0-5]

chmod $mode  /dev/${device}[0-9] /dev/${device}1[0-5]
