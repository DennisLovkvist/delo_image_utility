#!/bin/sh
IMAGES=$(ls *.png)

for IMAGE in $IMAGES
do
    FILE_INFO=$(identify -format "%wx%h" $IMAGE)
    IFS='x'
    read -a DIMENSIONS <<< "$FILE_INFO"
    WIDTH=${DIMENSIONS[0]}
    HEIGHT=${DIMENSIONS[1]}

    echo $WIDTH
    echo $HEIGHT

    N=300
    FACTOR=$(echo "$N / $HEIGHT" | bc -l)

    delo_img -op resize -i $IMAGE -s $FACTOR

done

IMAGES=$(ls resized_*)

for IMAGE in $IMAGES
do
    FILE_INFO=$(identify -format "%wx%h" $IMAGE)
    IFS='x'
    read -a DIMENSIONS <<< "$FILE_INFO"
    WIDTH=${DIMENSIONS[0]}
    HEIGHT=${DIMENSIONS[1]}

    echo $WIDTH
    echo $HEIGHT

    N=300
    FACTOR=$(echo "$N / $WIDTH" | bc -l)

    delo_img -op crop -i $IMAGE -u pct -x 0.25 -y 0 -w $FACTOR -h 1
    rm $IMAGE

done


