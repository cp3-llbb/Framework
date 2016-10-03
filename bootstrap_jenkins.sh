#! /bin/sh

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git cms-merge-topic ikrav:egm_id_80X_v1
git clone -o upstream https://github.com/bachtis/analysis.git -b KaMuCa_V4 KaMuCa 
pushd KaMuCa
git checkout 5652259bc6c66af7127d75d98ac90fd58871128e
popd
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
