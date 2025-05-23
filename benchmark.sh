#!/bin/sh

MIN_THREADS=1
MAX_THREADS=32

MIN_ACCESS_DISTANCE=2048
MAX_ACCESS_DISTANCE=16384

for (( threads=$MIN_THREADS; threads<=$MAX_THREADS; ++threads ))
do
    for (( access_distance=$MIN_ACCESS_DISTANCE; access_distance<=$MAX_ACCESS_DISTANCE; access_distance+=256 ))
    do
	./scheduling_benchmark $threads $access_distance
    done
done
