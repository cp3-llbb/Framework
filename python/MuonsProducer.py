import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('muons'),
        prefix = cms.string('muon_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('slimmedMuons'),
            applyRochester = cms.untracked.bool(False),
            rochesterInputFile = cms.untracked.FileInPath('cp3_llbb/Framework/data/RoccoR_13tev.txt'),
            ea_R03 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaMuons_cone03_pfNeuHadronsAndPhotons.txt'),
            ea_R04 = cms.untracked.FileInPath('cp3_llbb/Framework/data/effAreaMuons_cone04_pfNeuHadronsAndPhotons.txt'),
            scale_factors = cms.untracked.PSet(
                tracking = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_tracking_ratio_eta_2016_9p2.json'),
                id_loose  = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_LooseID_genTracks_id.json'),
                id_medium = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_MediumID_genTracks_id.json'),
                id_tight  = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_TightIDandIPCut_genTracks_id.json'),

                iso_loose_id_tight = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_LooseRelIso_TightID_iso.json'),
                iso_loosetk_id_tight = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_LooseRelTkIso_TightID_iso.json'),
                iso_tight_id_tight = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_TightRelIso_TightID_iso.json'),

                id_tight_hww_period1 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_TightID_Run2016_Run271036to275783_PTvsETA_HWW.json'),
                id_tight_hww_period2 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_TightID_Run2016_Run275784to276500_PTvsETA_HWW.json'),
                id_tight_hww_period3 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_TightID_Run2016_Run276501to276811_PTvsETA_HWW.json'),
                iso_tight_hww_period1 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_ISOTight_Run2016_Run271036to275783_PTvsETA_HWW.json'),
                iso_tight_hww_period2 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_ISOTight_Run2016_Run275784to276500_PTvsETA_HWW.json'),
                iso_tight_hww_period3 = cms.untracked.FileInPath('cp3_llbb/Framework/data/ScaleFactors/Muon_data_mc_ISOTight_Run2016_Run276501to276811_PTvsETA_HWW.json'),
                )
            )
        )
