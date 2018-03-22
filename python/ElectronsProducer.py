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
#            mva_id = cms.untracked.PSet(
#                values = cms.untracked.InputTag('electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Values'),
#                categories = cms.untracked.InputTag('electronMVAValueMapProducer:ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Categories')
#            ),
            scale_factors = cms.untracked.PSet(
              #                id_veto_moriond17 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_veto_moriond17.json'),
#                id_loose_moriond17 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_loose_moriond17.json'),
#                id_medium_moriond17 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_medium_moriond17.json'),
#                id_tight_moriond17 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_tight_moriond17.json'),
#                reco_moriond17 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_EGamma_SF2D_reco_moriond17.json'),
#
#                hww_wp = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_Tight_Run2016_Run273726to276811_HWW_weighted.json'),
#                hww_mva80_wp = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_mva_80p_Iso2016_HWW_Full2016.json'),
#                hww_mva90_wp = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electron_mva_90p_Iso2016_HWW_Full2016.json'),
#
                # The following scale factors are corrections for events at high eta and low pT, coming from:
                # https://github.com/latinos/LatinoAnalysis/blob/c62fa0d2fd603eed0001621eb9335afc6ca4cbcd/Gardener/python/variables/LeptonEtaPtCorrFactors.py#L132
#                hww_highEta_corr = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electrons_highEtaCorrection.json'),
#                hww_lowPt_corr = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Electrons_lowPtCorrection.json')
                )
            ),
        )
