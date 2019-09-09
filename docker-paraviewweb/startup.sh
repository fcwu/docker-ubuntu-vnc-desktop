#!/bin/bash

service nginx start

if [ -z "$DATASET_HID" ]
then
    Visualizer --paraview $CONDA/lib/paraview-5.2/ \
        --data /input \
        --port 8777 \
	--server-only
else
    Visualizer --paraview $CONDA/lib/paraview-5.2/ \
        --data /input \
        --port 8777 \
	--server-only \
        --load-file $DATASET_HID
fi
