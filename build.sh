#!/bin/sh
cd src/source
g++ -std=c++17 *.cxx -o ../ticketbot -ldpp
cd .. 
./ticketbot
