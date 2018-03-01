
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework
from cp3_llbb.Framework.CmdLine import CmdLine

options = CmdLine(defaults=dict(runOnData=0, era="25ns", globalTag='80X_mcRun2_asymptotic_2016_TrancheIV_v7'))

framework = Framework.Framework(options)

framework.addAnalyzer('dilepton', cms.PSet(
    type = cms.string('dilepton_analyzer'),
    prefix = cms.string('dilepton_'),
    enable = cms.bool(True),
    categories_parameters = cms.PSet(
        mll_cut = cms.untracked.double(20)
        ),
    parameters = cms.PSet(
        standalone = cms.untracked.bool(True),
        muons_wp = cms.untracked.string('loose'),
        electrons_wp = cms.untracked.string('loose')
        )
    ))


framework.addAnalyzer('test', cms.PSet(
    type = cms.string('test_analyzer'),
    prefix = cms.string('test_'),
    enable = cms.bool(True),
    parameters = cms.PSet(
        test = cms.untracked.string('jets'),
        test2 = cms.untracked.string('met')
        )
    ))

framework.removeProducer('fat_jets')


# Load JEC from the specified database instead of the GT. Will also affect the JEC systematics
# framework.useJECDatabase('Fall15_25nsV2_MC.db')

from cp3_llbb.Framework.JetsProducer import discriminators_deepFlavour
framework.redoJEC(addBtagDiscriminators=discriminators_deepFlavour)
# framework.smearJets()
# framework.smearJets(resolutionFile='cp3_llbb/Framework/data/Spring16_25nsV10_MC_PtResolution_AK4PFchs.txt', scaleFactorFile='cp3_llbb/Framework/data/Spring16_25nsV10_MC_SF_AK4PFchs.txt')
# framework.applyMuonCorrection("kamuca", tag="MC_80X_13TeV")
# framework.applyMuonCorrection("rochester", input="cp3_llbb/Framework/data/Rochester/2016.v3/config.txt")
# framework.applyElectronRegression()
# framework.applyElectronSmearing(tag='Moriond17_23Jan')

# framework.doSystematics(['jec', 'jer'])
# framework.doSystematics(['jec'], jec={'uncertaintiesFile': 'cp3_llbb/HHAnalysis/data/Summer16_23Sep2016V4_MC_UncertaintySources_AK4PFchs.txt', 'splitBySources': True})

# Change the pt cut for testing if it propagates correctly
# framework.getProducer('jets').parameters.cut = 'pt > 50'

process = framework.create()

process.source.fileNames = cms.untracked.vstring(
        # '/store/mc/RunIISummer16MiniAODv2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-madgraphMLM-herwigpp_30M/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/110000/043B3BC8-8BB7-E611-8CCE-0090FAA573E0.root'
        '/store/mc/RunIISummer16MiniAODv2/TTTo2L2Nu_TuneCUETP8M2_ttHtranche3_13TeV-powheg-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/00ED79D3-CFC1-E611-B748-3417EBE64483.root'
        )

# Only run on a specific event. Useful for debugging

# NaN b-tagging discriminant for a jet
# input file: /store/mc/RunIISpring15MiniAODv2/TTTo2L2Nu_13TeV-powheg/MINIAODSIM/74X_mcRun2_asymptotic_v2-v1/60000/88E6468A-C56D-E511-B6C8-001E67248142.root
#process.source.eventsToProcess = cms.untracked.VEventRange(
        #'1:25002:4987798',
        #)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(2000))
