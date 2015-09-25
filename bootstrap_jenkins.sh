#! /bin/sh

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git clone https://github.com/cms-jet/JetToolbox JMEAnalysis/JetToolbox
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
