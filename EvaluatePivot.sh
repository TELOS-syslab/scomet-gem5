#!/bin/bash

if [ -z "$1" ]; then
    echo "Test All..."
    ./scripts/testBE.sh
    ./scripts/Pivotsilo.sh &
    ./scripts/Pivotimgdnn.sh &
    ./scripts/Pivotmoses.sh &
    ./scripts/Pivotxapian.sh &
    ./scripts/Pivotmasstree.sh &
    ./scripts/MBAimgdnn.sh &
    ./scripts/MBAxapian.sh &
    ./scripts/MBAsilo.sh &
    ./scripts/MBAmoses.sh &
    ./scripts/MBAmasstree.sh &
    ./scripts/MPAMimgdnn.sh &
    ./scripts/MPAMsilo.sh &
    ./scripts/MPAMxapian.sh &
    ./scripts/MPAMmoses.sh &
    ./scripts/MPAMmasstree.sh &

    wait
    sleep 180
elif [ "$1" == "BE" ]; then
    echo "Test BE Baseline..."
    ./scripts/testBE.sh &

    wait
    sleep 120
elif [ "$1" == "Pivot" ]; then
    echo "Test Pivot..."
    ./scripts/Pivotimgdnn.sh &
    ./scripts/Pivotsilo.sh &
    ./scripts/Pivotxapian.sh &
    ./scripts/Pivotmoses.sh &
    ./scripts/Pivotmasstree.sh &

    wait
    sleep 180
elif [ "$1" == "MBA" ]; then
    echo "Test MBA..."
    ./scripts/MBAimgdnn.sh &
    ./scripts/MBAsilo.sh &
    ./scripts/MBAxapian.sh &
    ./scripts/MBAmoses.sh &
    ./scripts/MBAmasstree.sh &

    wait
    sleep 180
elif [ "$1" == "MPAM" ]; then
    echo "Test MPAM..."
    ./scripts/MPAMimgdnn.sh &
    ./scripts/MPAMsilo.sh &
    ./scripts/MPAMxapian.sh &
    ./scripts/MPAMmoses.sh &
    ./scripts/MPAMmasstree.sh &

    wait
    sleep 180
else
    echo "Wrong Argument: Please Use BE, MBA, MPAM or Pivot!"
fi

