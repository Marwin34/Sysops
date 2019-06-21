#!/usr/bin/env bash
rm -f times.txt
touch times.txt

make filter_generator
./filter_generator

make main

for method in 0 1
do
    if (( $method == 0 )) ; then
        printf "%s\n" " === BLOCK METHOD === " >> times.txt
    else
        printf "%s\n" " === INTERLEAVED METHOD === " >> times.txt
    fi
    
    for filter_size in $(seq 3 8 65)
    do
        printf " --- Kernel size: $filter_size --- \n" $ >> times.txt

        THREAD_MAX_POWER=3

        for thread_num_power in $(seq 0 $THREAD_MAX_POWER)
        do
            thread_num=$((2**$thread_num_power))
            output=$(./main $thread_num $method images/lena.ascii.pgm \
                $(printf "generated_filters/filter_%d.txt" "$filter_size") \
                $(printf "images_out/output_%d.pgm" "$filter_size") \
            | tail -1)
            value=${output#*:}
            printf "Threads num: %4d Time: %s \n" $thread_num "$value" >> times.txt
        done
    done
    printf "\n\n" >> times.txt
done