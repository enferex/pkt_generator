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

trap "echo " SIGINT

# Before test
sudo ethtool -S $dev > $dir/ethtool.before
sudo ifconfig $dev   > $dir/ifconfig.before

# Test
./pkt_generator -c -i $dev -b $pktsize

# After test
sudo ethtool -S $dev    > $dir/ethtool.after
sudo ifconfig $dev      > $dir/ifconfig.after
wc -l pkt_generator.log > $dir/userland_recv.count

mv pkt_generator.log $dir/${host}_recv.log
