import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('electrons'),
        prefix = cms.string('electron_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('phase2Electrons'),
            ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Summer16/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_80X.txt'),
            ea_R04 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaElectrons_cone04_pfNeuHadronsAndPhotons.txt'),
            ids = cms.untracked.VInputTag(
#                'No ids defined for phase2'
                ),
            scale_factors = cms.untracked.PSet(
                )
            ),
        )
