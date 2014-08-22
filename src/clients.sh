#!/bin/bash
declare -i s
for (( i=1; i <= 10*1024; i=i+1 )) 
    do 
    ./client &
    done
