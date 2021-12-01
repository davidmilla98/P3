#!/bin/bash

# Put here the program (maybe with path)

for up in 0.83 0.835; do
for thc in 0.001 0.0001 0.0005; do

GETF0="get_pitch --umbral=$up --clipping=$thc"


for fwav in pitch_db/train/*.wav; do
    ff0=${fwav/.wav/.f0}
    $GETF0 $fwav $ff0 > /dev/null || (echo "Error in $GETF0 $fwav $ff0"; exit 1)
done
done
done

echo "Parametros: Umbral: $up, TH_CLIPPING: $thc"
pitch_evaluate pitch_db/train/*f0ref

exit 0
