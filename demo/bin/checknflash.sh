#!/bin/bash

default_numfpgas=6
default_flash=0
default_image="./sancus-core-128bit-pm2uart-pm3p3-hatp1.mcs"

OPTIND=1  # for getopt

numfpgas=$default_numfpgas
fpgaid="-1"
flash=$default_flash

while getopts "hn:i:f:" opt; do
  case "$opt" in
    h)
       echo "Synopsis: $0 [-h] [-n numfpgas:int] [-i fpgaid:int] [-f flash:0|1]";
       exit 0;
       ;;
    n)
       numfpgas=$OPTARG;
       if [ $numfpgas -lt 0 ]; then
         echo "Bad numfpgas."; exit 1;
       fi
       ;;
    i)
       fpgaid=$OPTARG;
       if [ $fpgaid -lt 0 ]; then
         echo "Bad fpgaid."; exit 1;
       fi
       ;;
    f)
       flash=$OPTARG;
       if [ $flash -lt 0 ]; then
         echo "Bad flash."; exit 1;
       fi
       ;;
  esac
done


firstfpga=0
lastfpga=$(expr $numfpgas - 1)

if [ $fpgaid -ge 0 ]; then
  numfpgas=$fpgaid;
  firstfpga=$fpgaid;
  lastfpga=$fpgaid;
fi

for (( fpga=$firstfpga; fpga<=$lastfpga; fpga++ )); do
  echo -n "FPGA $fpga: ";
  xstest -u $fpga;
  if [ $flash -gt 0 ]; then
    echo -n "Flashing: ";
    xsload --usb $fpga -b xula2-lx25 --flash "$default_image";
  fi
done


