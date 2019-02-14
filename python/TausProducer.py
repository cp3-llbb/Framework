import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('taus'),
        prefix = cms.string('tau_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            src = cms.untracked.InputTag('slimmedTaus'),
            ),
        )
