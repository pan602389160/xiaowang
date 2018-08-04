#!/bin/bash
type=`grep  -A 1 "\[audio\]" /usr/data/system.ini | grep "type" |cut -d = -f 2`
volume -w 85
./aec_delay_test $1 $type
