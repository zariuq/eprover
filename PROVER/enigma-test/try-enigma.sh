#!/bin/sh

P=$1

if [ -z "$P" ]; then
   echo "usage: $0 problem.p"
   exit -1
fi

eprover `cat mzr01` -p -s --print-statistics --training-examples=3 ${P} > ${P}.out
grep Processed ${P}.out
grep trainpos ${P}.out > ${P}.pos
grep trainneg ${P}.out > ${P}.neg
eprover --cnf ${P} > ${P}.cnf
enigma-features ${P}.pos ${P}.neg ${P}.cnf > ${P}.pre
./enigma-make.py ${P}.pre

eprover `cat mzr01.enigma` -p -s --print-statistics --training-examples=3 ${P} | grep Processed

