#!/bin/bash

if [ $# == 0 ]; then
	echo "Usage: $0 <num packets> <packet size> <host> <local device name>"
	exit
fi

npkts=$1
pktsize=$2
host=$3
dev=$4
dir=tests/recv/${npkts}p_${pktsize}b/

echo "Creating directory $dir..."
mkdir -p $dir

trap "echo hi" SIGINT

sudo ethtool -S $dev > $dir/$dev.before;
./pkt_generator -c -b $pktsize
sudo ethtool -S $dev > $dir/$dev.after
mv pkt_generator.log $dir/${host}_recv.log
