#!/bin/bash

[ -f Spring16_25nsV1_MC.db ] || wget https://github.com/cms-jet/JECDatabase/raw/master/SQLiteFiles/Spring16_25nsV1_MC.db

# Input files
testfileshome="http://cp3.irmp.ucl.ac.be/~pdavid/llbbframeworktest"
[ -f DoubleMuon_Run2016B_PromptReco-v2_reduced.root ] || wget "${testfileshome}/DoubleMuon_Run2016B_PromptReco-v2_reduced.root"
[ -f TTTo2L2Nu_13TeV-powheg_RunIISpring16MiniAODv2_reduced.root ] || wget "${testfileshome}/TTTo2L2Nu_13TeV-powheg_RunIISpring16MiniAODv2_reduced.root"
