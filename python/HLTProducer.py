import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('hlt'),
        prefix = cms.string('hlt_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            hlt = cms.untracked.InputTag('TriggerResults', '', 'HLT'),
            triggers = cms.untracked.FileInPath('cp3_llbb/Framework/data/triggers.xml')
            )
        )
