#/bin/bash 

touch ODEBUG

cmake -DCMAKE_BUILD_TYPE=debug -DEXECUTABLE_OUTPUT_PATH=./bin/

