#!/bin/bash

make clean > /dev/null 2>&1

make > /dev/null 2>&1

HOST=`hostname`

ID=pma${HOST##R}

cp -rf host_config.xml.$ID etc/host_config.xml

