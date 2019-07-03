#!/bin/bash

for i in `seq 1 100`
do
    (
    ./client 127.0.0.1 6666 
    ) &
done

wait
echo hellworld
