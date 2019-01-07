import FWCore.ParameterSet.Config as cms

discriminators_deepFlavour = list("{algo}:prob{tagCat}".format(algo=algo, tagCat=tagCat)
        for algo in ("pfDeepCSVJetTags",)# "pfDeepCMVAJetTags" seems more difficult on miniAOD in CMSSW_8_0_30
        for tagCat in ("b", "bb", "c", "udsg")
        )

default_configuration = cms.PSet(
        type = cms.string('jets'),
        prefix = cms.string('jet_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            jets = cms.untracked.InputTag('slimmedJets'),
            cut = cms.untracked.string("pt > 10"),
            btags = cms.untracked.vstring('pfCombinedInclusiveSecondaryVertexV2BJetTags', 'pfCombinedMVAV2BJetTags', *discriminators_deepFlavour),
            scale_factors = cms.untracked.PSet(
                ## name=cms.untracked.PSet(
                ##     algorithm=cms.untracked.string(),
                ##     working_point=cms.untracked.string(),
                ##     files=cms.untracked.VPSet(
                ##         cms.untracked.PSet(
                ##             flavor=cms.untracked.string(),
                ##             file=cms.untracked.FileInPath(),
                ##             ),
                ##         )
                ##     )
                ),
            computeRegression = cms.untracked.bool(False),
            ## regressionFile = cms.untracked.FileInPath(),
            )
        )
