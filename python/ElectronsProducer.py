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
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-loose',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-medium',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight',
                ),
            scale_factors = cms.untracked.PSet(
                id_veto = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_veto.json'),
                id_loose = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_loose.json'),
                id_medium = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_medium.json'),
                id_tight = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_tight.json'),

                gsf_tracking = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_gsfTracking.json'),

                hww_wp = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_Tight_Run2016_Run273726to276811_HWW_weighted.json')
                )
            ),
        )
