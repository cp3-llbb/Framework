#! /bin/sh

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git cms-merge-topic ikrav:egm_id_80X_v2
git cms-merge-topic rafaellopesdesa:Regression80XEgammaAnalysis_v2
git cms-merge-topic gpetruc:badMuonFilters_80X_v2

git clone -o upstream https://github.com/bachtis/analysis.git -b KaMuCa_V4 KaMuCa 
pushd KaMuCa
git checkout 2ad38daae37a41a9c07f482e95f2455e3eb915b0
popd
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
