#!/bin/bash

for i in `seq 1 100`
do
    (
    ./client 127.0.0.1 9999 
    ) &
done

wait
echo hellworld
