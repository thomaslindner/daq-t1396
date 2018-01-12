#!/bin/bash
# midas2root script
# T. Lindner

source /home/nfs/ftbf_user/midas-daq/midas_minimal.sh
# Go to right output directory
cd /data/t1396/root_files

# Get current filename
export Filename=`odbedit -c 'ls "/Logger/Channels/0/Settings/Current filename"' | awk -F "  +" '{print $2}'`


# Set midas file directory
export MidasDir=/data/t1396/midas_data/

# Do the conversion
/home/nfs/ftbf_user/midas-daq/daq-t1396/analyzer/midas2root.exe $MidasDir/$Filename


