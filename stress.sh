> stress.txt
for (( N=1 ; N<=50 ; N++ ));
do
    echo "N: $N" >> stress.txt
    if [[ $((N%2)) == 0 ]]; then
        for (( i=0 ; i<N ; i++ ));
        do
            for (( j=0 ; j<N ; j++ ));
            do
                echo "$N $i $j:" >> stress.txt
                ./a1.out $N $i $j | head -c 20 >> stress.txt
                echo "" >> stress.txt
            done
        done
    else
        for (( i=0 ; i<N ; i++ ));
        do
            for (( j=0 ; j<N ; j++ ));
            do
                x=i+j
                if [[ $((x%2)) == 0 ]]; then
                echo "$N $i $j:" >> stress.txt
                ./a1.out $N $i $j | head -c 20 >> stress.txt
                echo "" >> stress.txt
                fi
            done
        done
    fi
done