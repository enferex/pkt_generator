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

sudo ethtool -S $dev > $dir/$dev.before
./pkt_generator -s $host -p $npkts -b $pktsize 
sudo ethtool -S $dev > $dir/$dev.after
mv pkt_generator.log $dir/send_to_${host}.log
