#!/bin/bash
declare -i s
for (( i=1; i <= 100; i=i+1 )) 
    do 
    ./client &
    done
