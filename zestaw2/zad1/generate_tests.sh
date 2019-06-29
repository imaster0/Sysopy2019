#!/bin/bash

echo '' > logs # clear logs

SMALL=200
LARGE=1000
#make dir if doesnt exist
mkdir -p test_files

for i in 1 4 512 1024 4096 8192
do
    #generate
    ./main generate test_files/test${i}_1 $SMALL $i
    ./main generate test_files/test${i}_2 $LARGE $i
    echo "#TESTS GENERATED" >> logs
    #copy
    echo "##copy" >> logs
    echo "### small test sys ${i}B" >> logs
    ./main copy test_files/test${i}_1 test_files/test${i}_1sys $SMALL $i sys
    echo "### large test sys ${i}B" >> logs
    ./main copy test_files/test${i}_2 test_files/test${i}_2sys $LARGE $i sys
    echo "### small test lib ${i}B" >> logs
    ./main copy test_files/test${i}_1 test_files/test${i}_1lib $SMALL $i lib
    echo "### large test lib ${i}B" >> logs
    ./main copy test_files/test${i}_2 test_files/test${i}_2lib $LARGE $i lib
    #sort
    echo "##sort" >> logs
    echo "### small test sys ${i}B" >> logs
    ./main sort test_files/test${i}_1sys $SMALL $i sys
    echo "### large test sys ${i}B" >> logs
    ./main sort test_files/test${i}_2sys $LARGE $i sys
    echo "### small test lib ${i}B" >> logs
    ./main sort test_files/test${i}_1lib $SMALL $i lib
    echo "### large test lib ${i}B" >> logs
    ./main sort test_files/test${i}_2lib $LARGE $i lib
done

