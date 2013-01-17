#!/bin/bash

if [ $# == 0 ]; then
	echo "Usage: $0 <num packets> <packet size> <send to> <local device name>"
	exit
fi

npkts=$1
pktsize=$2
host=$3
dev=$4
dir=tests/send/${npkts}p_${pktsize}b/

echo "Creating directory $dir..."
mkdir -p $dir

# Before test
sudo ethtool -S $dev > $dir/ethtool.before
sudo ifconfig $dev   > $dir/ifconfig.before

# Test
./pkt_generator -s $host -i $dev -p $npkts -b $pktsize 

# After test
sudo ethtool -S $dev    > $dir/ethtool.after
sudo ifconfig $dev      > $dir/ifconfig.after
wc -l pkt_generator.log > $dir/userland_send.count

mv pkt_generator.log $dir/send_to_${host}.log
