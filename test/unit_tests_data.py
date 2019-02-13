
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework
from cp3_llbb.Framework.CmdLine import CmdLine

options = CmdLine(defaults=dict(runOnData=1, era="2016", globalTag='94X_dataRun2_v10', process="RECO"))

framework = Framework.Framework(options)

process = framework.create()

process.source.fileNames = cms.untracked.vstring(
        "/store/data/Run2016B/DoubleMuon/MINIAOD/17Jul2018_ver2-v1/00000/F0736B77-E38B-E811-BED7-3417EBE528B2.root"
        )

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))
