#!/usr/bin/sh

for i in `find tests/ -name '*.log'; do wc -l $i; done
