
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework
from cp3_llbb.Framework.CmdLine import CmdLine

options = CmdLine()
options.changeDefaults(runOnData=0, era="25ns", globalTag='80X_mcRun2_asymptotic_2016_miniAODv2_v1')

framework = Framework.Framework(options)
framework.redoJEC(JECDatabase='Spring16_25nsV1_MC.db')
process = framework.create()

process.source.fileNames = cms.untracked.vstring(
        'file://TTTo2L2Nu_13TeV-powheg_RunIISpring16MiniAODv2_reduced.root'
        )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))
