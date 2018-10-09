import FWCore.ParameterSet.Config as cms

default_configuration = cms.PSet(
        type = cms.string('met'),
        prefix = cms.string('met_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            met = cms.untracked.InputTag('slimmedMETs'),
            metCov = cms.untracked.InputTag("METSignificance","METCovariance")
            )
        )
