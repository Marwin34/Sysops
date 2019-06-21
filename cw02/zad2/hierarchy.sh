#!/bin/sh


mkdir top
cd top
mkdir first
mkdir second
touch -t 1902011456 third
touch fourth
cd first
mkdir fish
mkdir cat
touch -t 1903102155 dog
ln -s ../second crab
touch -t 1902011532 cow
touch lynx
cd fish
touch -t 1902101645 orange 
mkdir apple
cd apple
mkdir tomato
touch -t 1902061856 potato
ln -s ../orange carrot
cd ..
cd ..
cd cat
touch fed_him
ln -s ../fish purrina
cd ..
cd ..
cd second
touch -t 1903051444 nothing
cd ..
touch -t 1902111332 second
cd first 
touch -t 1903141312 fish
cd fish
touch -t 1902131456 apple
