#!/bin/bash

if [ -d thirdparty/.git ]; then
  (cd thirdparty && git pull --rebase)
elif [ -d thirdparty ]; then
  echo "The thirdparty directory exists but isn't a git repo."
  echo "Please remove it and try again"
  exit 2
else
  git clone https://github.com/SCIInstitute/map3d-thirdparty.git thirdparty
fi

