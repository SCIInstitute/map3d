#!/bin/bash

if [ -d thirdparty ]; then
  (cd thirdparty && git pull --rebase)
else
  git clone https://github.com/SCIInstitute/map3d-thirdparty.git thirdparty
fi

