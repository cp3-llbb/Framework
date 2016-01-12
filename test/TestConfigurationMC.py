
import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras
from cp3_llbb.Framework import Framework

process = Framework.create(False, eras.Run2_25ns, '74X_mcRun2_asymptotic_v2', cms.PSet(
    dilepton = cms.PSet(
        type = cms.string('dilepton_analyzer'),
        prefix = cms.string('dilepton_'),
        enable = cms.bool(True),
        categories_parameters = cms.PSet(
            mll_cut = cms.untracked.double(20)
            ),
        parameters = cms.PSet(
            standalone = cms.untracked.bool(True),
            muons_wp = cms.untracked.string('loose'),
            electrons_wp = cms.untracked.string('loose')
            )
        ),

    bTagsLoose = cms.PSet(
        type = cms.string('btags_analyzer'),
        prefix = cms.string('btags_CSVv2_loose'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            discr_name = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            discr_cut = cms.untracked.double(0.605),
            eta_cut = cms.untracked.double(2.4),
            pt_cut = cms.untracked.double(30)
            )
        ),

    bTagsMedium = cms.PSet(
        type = cms.string('btags_analyzer'),
        prefix = cms.string('btags_CSVv2_medium'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            discr_name = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            discr_cut = cms.untracked.double(0.89),
            eta_cut = cms.untracked.double(2.4),
            pt_cut = cms.untracked.double(30)
            )
        ),

    bTagsTight = cms.PSet(
        type = cms.string('btags_analyzer'),
        prefix = cms.string('btags_CSVv2_tight'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            discr_name = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            discr_cut = cms.untracked.double(0.97),
            eta_cut = cms.untracked.double(2.4),
            pt_cut = cms.untracked.double(30)
            )
        ),

    test = cms.PSet(
        type = cms.string('test_analyzer'),
        prefix = cms.string('test_'),
        enable = cms.bool(True),
        parameters = cms.PSet(
            test = cms.untracked.string('jets'),
            test2 = cms.untracked.string('met')
            )
        )
    ), 
    
    redoJEC=False,

    doSystematics=['jec', 'jer']
    )

#Framework.schedule(process, analyzers=['dilepton', 'bTagsLoose', 'bTagsMedium', 'bTagsTight', 'test'],
        #producers=['event', 'gen_particles', 'hlt', 'vertices', 'electrons', 'muons', 'jets', 'fat_jets', 'met', 'nohf_met'])

process.source.fileNames = cms.untracked.vstring(
        '/store/mc/RunIISpring15MiniAODv2/TTJets_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/74X_mcRun2_asymptotic_v2-v1/00000/0014DC94-DC5C-E511-82FB-7845C4FC39F5.root'
        )

# Only run on a specific event. Useful for debugging

# NaN b-tagging discriminant for a jet
# input file: /store/mc/RunIISpring15MiniAODv2/TTTo2L2Nu_13TeV-powheg/MINIAODSIM/74X_mcRun2_asymptotic_v2-v1/60000/88E6468A-C56D-E511-B6C8-001E67248142.root
#process.source.eventsToProcess = cms.untracked.VEventRange(
        #'1:25002:4987798',
        #)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(400))
