#!/bin/sh
for IMAGE in *.png;
do
    FILE_INFO=$(identify -format "%wx%h" $IMAGE)
    IFS='x'
    read -a DIMENSIONS <<< "$FILE_INFO"
    WIDTH=${DIMENSIONS[0]}
    HEIGHT=${DIMENSIONS[1]}
	N=300

    if [ $WIDTH -gt $HEIGHT ]
    then
        FACTOR=$(echo "$N / $HEIGHT" | bc -l) 
    else
        FACTOR=$(echo "$N / $WIDTH" | bc -l)
    fi

	delo_img -op resize -i $IMAGE -s $FACTOR
	

    
    

done


for IMAGE in resized_*;
do
    FILE_INFO=$(identify -format "%wx%h" $IMAGE)
    IFS='x'
    read -a DIMENSIONS <<< "$FILE_INFO"
    WIDTH=${DIMENSIONS[0]}
    HEIGHT=${DIMENSIONS[1]}

    N=300
	
    if [ $WIDTH -ne $HEIGHT ]
    then

        if [ $WIDTH -gt $HEIGHT ]
        then
            FACTOR=$(echo "$N / $WIDTH" | bc -l)
            delo_img -op crop -i $IMAGE -u pct -x 0 -y 0 -w $FACTOR -h 1
        else
            FACTOR=$(echo "$N / $HEIGHT" | bc -l)
            delo_img -op crop -i $IMAGE -u pct -x 0 -y 0 -w 1 -h $FACTOR
        fi  
    else
        delo_img -op crop -i $IMAGE -u pct -x 0 -y 0 -w 1 -h 1
    fi 
	
    rm $IMAGE

done


