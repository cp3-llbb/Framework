#! /bin/sh

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git cms-merge-topic ikrav:egm_id_80X_v1
git clone -o Analysis https://github.com/bachtis/analysis.git -b KaMuCa_V4 KaMuCa
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
