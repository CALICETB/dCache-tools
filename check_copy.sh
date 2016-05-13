#!/bin/bash

echo "Starting dCache Copy tool"

#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/Electrons_Showers/20160509/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/LED/20160511/"
DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/MIP_newDIF/20160513/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/MIP/20160505/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/PowerPulsing/no_powerpulsing/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/PowerPulsing/powerpulsing/"

########################### EUDAQ ###########################################

#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/eudaq-1.4.5-TestBeamCern/data/CERN_SPS_August_2015/Muons/120GeV/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/eudaq-1.4.5-TestBeamCern/data/CERN_SPS_August_2015/Pions/10GeV/"

#############################################################################

LOCAL_DIR="/home/calice/Desktop/dCache/script/auto"

ROOT_DCACHE="/grid/calice/"
OUTDIR_BASE="tb-desy/native/desyAhcal2016/"
OUTDIR_TAG="AHCAL_Testbeam_Raw_May_2016/"

#DIR="PowerPulsing/powerpulsing/"
#DIR="PowerPulsing/no_powerpulsing/"
#DIR="MIP/20160505/"
DIR="MIP_newDIF/20160513/"
#DIR="LED/20160511/"
#DIR="Electron_Showers/20160509/"

####################################################

echo "Replacing pathes in src file"
cp -vi check_copy.cpp check_copy_temp.cpp

/bin/vi check_copy_temp.cpp << EOF >& /dev/null
:%s#INDAQ#$DAQ_DIR#g
:%s#INLOCAL#$LOCAL_DIR#g
:%s#ROOT#$ROOT_DCACHE#g
:%s#BASE#$OUTDIR_BASE#g
:%s#TAG#$OUTDIR_TAG#g
:%s#OUTDIR#$DIR#g
:w!
:q
EOF

echo "Building..."

rm check_copy
g++ -o check_copy check_copy_temp.cpp

echo "Starting tool"
killall check_copy
nohup ./check_copy > check_log.txt &
