#!/bin/bash

if [ ! -d "Resource" ]; then
    echo "Downloading Pivot Resource..."
    curl -L -o Resource.tar.gz https://zenodo.org/record/14275908/files/Pivot-Resource.tar.gz
    tar -xvzf Resource.tar.gz
fi

python3 $(which scons) build/ARM/gem5.opt -j16 --install-hooks
