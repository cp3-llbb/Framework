
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework
from cp3_llbb.Framework.CmdLine import CmdLine

options = CmdLine(defaults=dict(runOnData=0, era="2016", globalTag='94X_mcRun2_asymptotic_v3'))

framework = Framework.Framework(options)
framework.redoJEC(JECDatabase='Spring16_25nsV1_MC.db')
process = framework.create()

process.source.fileNames = cms.untracked.vstring(
        '/store/mc/RunIISummer16MiniAODv3/TTTo2L2Nu_TuneCUETP8M2_ttHtranche3_13TeV-powheg-pythia8/MINIAODSIM/PUMoriond17_94X_mcRun2_asymptotic_v3-v2/270000/248C6048-5FEE-E811-80C4-0025902D144A.root'
        )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))
