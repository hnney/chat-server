#!/bin/bash


cnt=$#
#for((i=1;i<=${cnt};i++))
#do
 #    filename=`eval echo "\\\$$i"`
  #   scp -P 23 hjh@192.168.1.208:/home/hjh/practice/test/eventtest/${filename} .
#done

scp -P 23 hjh@192.168.1.208:/home/hjh/practice/test/eventtest/* .


