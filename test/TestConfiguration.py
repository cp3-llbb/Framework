
import FWCore.ParameterSet.Config as cms

process = cms.Process("ETM")

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff")
process.load("Configuration.EventContent.EventContent_cff")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')

#process.GlobalTag.globaltag = "MCRUN2_74_V9"
process.GlobalTag.globaltag = "74X_dataRun2_Express_v0"

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))
process.source = cms.Source("PoolSource")
process.source.fileNames = cms.untracked.vstring(
#        'file:///home/fynu/sbrochet/storage/MINIAODSIM/TTJets_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8_Asympt25ns_MCRUN2_74_V9_reduced.root'
'/store/data/Run2015B/DoubleMuon/MINIAOD/PromptReco-v1/000/251/244/00000/E42FEF61-6E27-E511-B93A-02163E0143C0.root',
'/store/data/Run2015B/DoubleMuon/MINIAOD/PromptReco-v1/000/251/251/00000/0292F6F9-8A27-E511-9074-02163E012213.root',
'/store/data/Run2015B/DoubleMuon/MINIAOD/PromptReco-v1/000/251/252/00000/9ADEE140-9C27-E511-919A-02163E011D23.root',
        )
##### #####
# Support for json files for data
##### #####
import FWCore.PythonUtilities.LumiList as LumiList
process.source.lumisToProcess = LumiList.LumiList(filename = 'Cert_246908-251252_13TeV_PromptReco_Collisions15_JSON.txt').getVLuminosityBlockRange()
##### #####
# Electron ID recipe
##### #####
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
dataFormat = DataFormat.MiniAOD
switchOnVIDElectronIdProducer(process, dataFormat)
# define which IDs we want to produce
my_id_modules = ['RecoEgamma.ElectronIdentification.Identification.mvaElectronID_PHYS14_PU20bx25_nonTrig_V1_cff','RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_PHYS14_PU20bx25_V2_cff', 'RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV51_cff']
#add them to the VID producer
for idmod in my_id_modules:
    setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

##### #####
# Services
##### #####
process.load('FWCore.MessageLogger.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100

# Producers
from cp3_llbb.Framework import EventProducer
#from cp3_llbb.Framework import GenParticlesProducer
from cp3_llbb.Framework import HLTProducer
from cp3_llbb.Framework import JetsProducer
from cp3_llbb.Framework import METProducer
from cp3_llbb.Framework import MuonsProducer
from cp3_llbb.Framework import ElectronsProducer
from cp3_llbb.Framework import VerticesProducer

process.framework = cms.EDProducer("ExTreeMaker",
        output = cms.string('output_mc.root'),

        producers = cms.PSet(

            event = EventProducer.default_configuration,

            hlt = HLTProducer.default_configuration,

#            gen_particles = GenParticlesProducer.default_configuration,

            jets = JetsProducer.default_configuration.clone(
                parameters = cms.PSet(
                    cut = cms.untracked.string("pt > 10")
                    )
                ),

            met = METProducer.default_configuration,

            muons = MuonsProducer.default_configuration,

            electrons = ElectronsProducer.default_configuration,

            vertices = VerticesProducer.default_configuration,
            ),

        analyzers = cms.PSet(
            test = cms.PSet(
                type = cms.string('test_analyzer'),
                prefix = cms.string('test_'),
                enable = cms.bool(True)
                )
            )
        )

process.p = cms.Path(
        process.egmGsfElectronIDSequence *
        process.framework
        )


#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#! Output and Log
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
process.options   = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))
process.options.allowUnscheduled = cms.untracked.bool(True)
