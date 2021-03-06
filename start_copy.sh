#!/bin/bash

echo "Starting dCache Copy tool"

DEBUG="false"

#############################################################################

#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/Electrons_Showers/20160516/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/LED/20160516/"
DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/MIP_newDIF/20160513/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/holdscan/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/MIP/20160505/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/PowerPulsing/no_powerpulsing/"
#DAQ_DIR="/home/calice/Desktop/DAQ_PC/DESY_May_2016/PowerPulsing/powerpulsing/"

########################### EUDAQ ###########################################

#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/Electron_showers/20160516/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/MIP_newDIF/20160513/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/MIP/20160505/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/PowerPulsing/powerpulsing/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/PowerPulsing/no_powerpulsing/"
#DAQ_DIR="/home/calice/Desktop/EUDAQ_PC/home/calice/EUDAQ1.6-TestBeamMay2016/data/"

#############################################################################

LOCAL_DIR="/home/calice/Desktop/dCache/script/auto"

ROOT_DCACHE="/grid/calice/"
OUTDIR_BASE="tb-desy/native/desyAhcal2016/"
OUTDIR_TAG="AHCAL_Testbeam_Raw_May_2016/"

#############################################################################

#DIR="PowerPulsing/powerpulsing/"
#DIR="PowerPulsing/no_powerpulsing/"
#DIR="MIP/20160505/"
DIR="MIP_newDIF/20160513/"
#DIR="LED/20160516/"
#DIR="holdscan/"
#DIR="Electron_Showers/20160516/"

############################## EUDAQ ######################################

#DIR="PowerPulsing/EUDAQ/powerpulsing/"
#DIR="PowerPulsing/EUDAQ/no_powerpulsing/"
#DIR="MIP/EUDAQ/20160505/"
#DIR="MIP_newDIF/EUDAQ/20160513/"
#DIR="LED/20160516/"
#DIR="holdscan/"
#DIR="Electron_Showers/EUDAQ/20160516/"
#DIR="raw/"

############### IF LED files #######################
LED="false"

############### IF EUDAQ files #####################
EUDAQ="false"

############### IF BIF files #####################
BIF="false"

####################################################

echo "Replacing pathes in src file"
cp -vi cp_hcal.cpp cp_hcal_temp.cpp

/bin/vi cp_hcal_temp.cpp << EOF >& /dev/null
:%s#INDAQ#$DAQ_DIR#g
:%s#INLOCAL#$LOCAL_DIR#g
:%s#ROOT#$ROOT_DCACHE#g
:%s#BASE#$OUTDIR_BASE#g
:%s#TAG#$OUTDIR_TAG#g
:%s#OUTDIR#$DIR#g
:%s#BOOLLED#$LED#g
:%s#BOOLEUDAQ#$EUDAQ#g
:%s#BOOLBIF#$BIF#g
:%s#VERBOSE#$DEBUG#g
:w!
:q
EOF

echo "Building..."

rm cp_hcal
g++ -o cp_hcal cp_hcal_temp.cpp

echo "Starting tool"
killall cp_hcal
#killall lcg-cr
killall gfal-copy
nohup ./cp_hcal > copy_log.txt &
