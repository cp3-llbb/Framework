import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('electrons'),
        prefix = cms.string('electron_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('slimmedElectrons'),
            ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/PHYS14/effAreaElectrons_cone03_pfNeuHadronsAndPhotons.txt'),
            ea_R04 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaElectrons_cone04_pfNeuHadronsAndPhotons.txt'),
            ids = cms.untracked.VInputTag(
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-loose',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-medium',
                'egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight',
                ),
            scale_factors = cms.untracked.PSet(
                id_veto = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_CutBasedID_VetoWP_fromTemplates_withSyst_76X.json'),
                id_loose = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_CutBasedID_LooseWP_fromTemplates_withSyst_76X.json'),
                id_medium = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_CutBasedID_MediumWP_fromTemplates_withSyst_76X.json'),
                id_tight = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_CutBasedID_TightWP_fromTemplates_withSyst_76X.json'),

                hww_wp = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electrons_HWW_CutBasedID_TightWP_fromTemplates_withSyst_v1_reco_id_iso.json'),
                )
            ),
        )
