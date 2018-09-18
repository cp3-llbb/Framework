import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('electrons'),
        prefix = cms.string('electron_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('slimmedElectrons'),
            ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Summer16/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_80X.txt'),
            ea_R04 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaElectrons_cone04_pfNeuHadronsAndPhotons.txt'),
            ids = cms.untracked.VInputTag(
                cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Fall17-94X-V2-veto"),
                cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Fall17-94X-V2-loose"),
                cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Fall17-94X-V2-medium"),
                cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Fall17-94X-V2-tight")
                ),
            mva_id = cms.untracked.PSet(
                values=cms.untracked.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Fall17IsoV1Values"),
                categories=cms.untracked.InputTag("electronMVAValueMapProducer:ElectronMVAEstimatorRun2Fall17IsoV1Categories")),
            scale_factors = cms.untracked.PSet(
                ## name=cms.untracked.FileInPath(),
                )
            ),
        )
