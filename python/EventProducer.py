import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('event'),
        prefix = cms.string('event_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            pu_summary = cms.untracked.InputTag('slimmedAddPileupInfo'),
            compute_pu_weights = cms.untracked.bool(True),
            pu_profile = cms.untracked.string("Spring16_25ns"),
            pu_reweighter = cms.PSet(
                    data_pu_profile = cms.untracked.FileInPath('cp3_llbb/Framework/data/PUProfiles/pu_histogram_2016_271036-284044_ReReco_69200minbias.root'),
                ),
            pu_reweighter_up = cms.PSet(
                    data_pu_profile = cms.untracked.FileInPath('cp3_llbb/Framework/data/PUProfiles/pu_histogram_2016_271036-284044_ReReco_72660minbias.root')
                ),
            pu_reweighter_down = cms.PSet(
                    data_pu_profile = cms.untracked.FileInPath('cp3_llbb/Framework/data/PUProfiles/pu_histogram_2016_271036-284044_ReReco_65740minbias.root')
                )
            )
        )
