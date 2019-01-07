
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework
from cp3_llbb.Framework.CmdLine import CmdLine

options = CmdLine(defaults=dict(runOnData=0, era="25ns", globalTag='80X_mcRun2_asymptotic_2016_miniAODv2_v1'))

framework = Framework.Framework(options)
from cp3_llbb.Framework.JetsProducer import discriminators_deepFlavour
framework.redoJEC(addBtagDiscriminators=discriminators_deepFlavour)
framework.smearJets()
#framework.applyMuonCorrection("kamuca", tag="MC_80X_13TeV")
##framework.applyElectronRegression()
##framework.applyElectronSmearing(tag='Moriond17_23Jan')
framework.doSystematics(['jec', 'jer'])

process = framework.create()

import os
v1,v2,v3 = tuple(int(tok) for tok in os.getenv("CMSSW_VERSION").split("_")[1:4])
if v1 > 9 or ( v1 == 8 and v2 == 0 and v3 > 30 ) or ( v1 == 9 and v2 >= 3 ):
    ## Don't use the deterministic seed yet
    ## for agreement between >=CMSSW_8_0_31 and <=CMSSW_8_0_30 (see PR#20241)
    ## and between >=CMSSW_9_3_0 and earlier CMSSW_9_X (see PR#20240)
    framework.process.slimmedJetsSmeared.useDeterministicSeed = cms.bool(False)
    framework.process.selectedUpdatedPatJetsAK4PFchsNewJECJERUp.useDeterministicSeed = cms.bool(False)
    framework.process.selectedUpdatedPatJetsAK4PFchsNewJECJERDown.useDeterministicSeed = cms.bool(False)

process.source.fileNames = cms.untracked.vstring(
        'file://TTTo2L2Nu_13TeV-powheg_RunIISpring16MiniAODv2_reduced.root'
        )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))
