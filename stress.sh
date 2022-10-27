file="./stress.txt"
for (( N=1 ; N<=50 ; N++ ));
do
    if [[ $((N%2)) == 0 ]]; then
        for (( i=0 ; i<N ; i++ ));
        do
            for (( j=0 ; j<N ; j++ ));
            do
                echo "$N $i $j:"
                ./a1.out $N $i $j
                echo ""
            done
        done
    else
        for (( i=0 ; i<N ; i++ ));
        do
            for (( j=0 ; j<N ; j++ ));
            do
                if [[  ]]
                echo "$N $i $j:"
                ./a1.out $N $i $j
                echo ""
            done
        done
    fi
done