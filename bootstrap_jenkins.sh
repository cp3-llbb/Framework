#! /bin/sh

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git cms-merge-topic ikrav:egm_id_7.4.12_v1
git cms-addpkg DataFormats/PatCandidates
git remote add blinkseb https://github.com/blinkseb/cmssw.git
git fetch blinkseb
git cherry-pick 794364a7352f045d7eed1fcc166fb92ece8ce68f
git cms-merge-topic blinkseb:jer_gt_74x_fix
git cms-merge-topic blinkseb:74x_smeared_jet_producer
git clone https://github.com/cms-jet/JetToolbox JMEAnalysis/JetToolbox
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
