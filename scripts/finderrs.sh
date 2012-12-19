#!/bin/sh

for i in `find tests/ -name '*.log'`; do wc -l $i; done

echo ""

for i in `find tests/ -name '[0-9]*p_[0-9]*b' -type d`
do
	diff $i/*.before $i/*.after | grep -ie "\(error\|fail\|loss\|drop\)"
done
