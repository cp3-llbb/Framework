import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('muons'),
        prefix = cms.string('muon_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('slimmedMuons'),
            ea_R03 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaMuons_cone03_pfNeuHadronsAndPhotons.txt'),
            ea_R04 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaMuons_cone04_pfNeuHadronsAndPhotons.txt'),
            scale_factors = cms.untracked.PSet(
                ## name=cms.untracked.FileInPath(),
                )
            )
        )
