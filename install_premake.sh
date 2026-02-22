#!/bin/bash

curl -L -o premakezip.tar.gz \
  https://github.com/premake/premake-core/releases/download/v5.0.0-beta8/premake-5.0.0-beta8-linux.tar.gz

tar -xzf premakezip.tar.gz

sudo mv premake5 /bin/

rm premakezip.tar.gz

sudo chmod +x /bin/premake5

echo "premake5 has been installed!"