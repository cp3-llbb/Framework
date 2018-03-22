import copy

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.SequenceTypes import _SequenceCollection

from Configuration.StandardSequences.Eras import eras

from cp3_llbb.Framework.deptracker import deptracker as dep
from cp3_llbb.Framework.Tools import change_process_name, change_input_tags_and_strings, StdStreamSilenter
from cp3_llbb.Framework import Tools
import cp3_llbb.Framework.Systematics as Systematics

class Framework(object):

    def __init__(self, options, verbose=True):
        self.__systematics = []
        self.__systematicsOptions = {}

        # Default configuration
        self.__miniaod_jet_collection = 'slimmedJets'
        self.__miniaod_unsmeared_jet_collection = 'slimmedJets'
        self.__miniaod_fat_jet_collection = 'slimmedJetsAK8'
        self.__miniaod_gen_jet_collection = 'slimmedGenJets'
        self.__miniaod_met_collection = 'slimmedMETs'
        self.__miniaod_muon_collection = 'slimmedMuons'
        self.__miniaod_electron_collection = 'slimmedElectrons'

        self.__jer_resolution_file = None
        self.__jer_scalefactor_file = None

        self.__kamuca_tag = 'DATA_80X_13TeV' if options.runOnData else 'MC_80X_13TeV'
        self.__rochester_input = 'cp3_llbb/Framework/data/Rochester/2016.v3/config.txt'

        self.hltProcessName = options.hltProcessName
        self.isData = options.runOnData
        self.era = options.era
        self.processName = options.process
        self.globalTag = options.globalTag
        self.verbose = verbose
        self.output_filename = 'output_data.root' if options.runOnData else 'output_mc.root'
        self.producers = []
        self.analyzers = []

        Tools.verbosity = self.verbose

        if self.globalTag is None:
            raise Exception("No global tag specified. Use the 'globalTag' command line argument to set one.\n\tcmsRun <configuration> globalTag=<global_tag>")

        if self.era is None:
            raise Exception("No era specified. Use the 'era' command line argument to set one.\n\tcmsRun <configuration> era=[25ns|50ns|2016]")

        if self.verbose:
            print("")
            print("CP3 llbb framework:")
            print("    - Running over %s" % ("data" if self.isData else "simulation"))
            print("    - global tag: %s" % self.globalTag)
            print("    - era: %s" % {eras.Run2_25ns:"25ns", eras.Run2_50ns:"50ns", eras.Run2_2016:"2016"}[self.era])
            print("")

        # Create CMSSW process and configure it with sane default values

        process = cms.Process("ETM", self.era)
        self.process = process
        self.path = cms.Path()

        # Workaround a bug in cms.Path.insert, where _seq is None and no check is done to create it
        if self.path._seq is None:
            self.path.__dict__["_seq"] = _SequenceCollection()

        process.options = cms.untracked.PSet(
                wantSummary = cms.untracked.bool(True),
                allowUnscheduled = cms.untracked.bool(True)
                )

        # Create an empty PSet for communication with GridIn
        process.gridin = cms.PSet(
                input_files = cms.vstring()
                )

        process.load('FWCore.MessageLogger.MessageLogger_cfi')
        process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
        process.load('Configuration.StandardSequences.MagneticField_cff')
        process.load('Configuration.Geometry.GeometryExtended2023D17Reco_cff')

        process.GlobalTag.globaltag = self.globalTag

        process.MessageLogger.cerr.FwkReport.reportEvery = 1000
        process.MessageLogger.suppressWarning = cms.untracked.vstring('framework')

        process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(20))
        process.source = cms.Source("PoolSource")

        self._configureFramework()

    @dep(before="create", performs="create")
    def create(self):
        """
        Terminate the process configuration and return the final CMSSW process
        """

        self.configureElectronId_()

        # Change process name if needed
        if self.processName is not None and self.processName != 'PAT':
            if self.verbose:
                print("")
                print("Changing process name from %r to %r..." % ('PAT', self.processName))
            change_process_name(self.process.framework, 'PAT', self.processName)

        if self.hltProcessName is not None and self.hltProcessName != 'HLT':
            if self.verbose:
                print("")
                print("Changing hlt process name from %r to %r..." % ('HLT', self.hltProcessName))
            change_process_name(self.process.framework, 'HLT', self.hltProcessName)

        if len(self.__systematics) > 0:
            if self.verbose:
                print("")

            default_systematics_options = {
                    'jec': {'jetCollection': self.__miniaod_jet_collection,
                        'metCollection': self.__miniaod_met_collection,
                        'uncertaintiesFile': None},
                    'jer': {'jetCollection': self.__miniaod_unsmeared_jet_collection,
                        'metCollection': self.__miniaod_met_collection,
                        'genJetCollection': self.__miniaod_gen_jet_collection}
                    }

            if self.__jer_resolution_file and self.__jer_scalefactor_file:
                default_systematics_options['jer']['resolutionFile'] = self.__jer_resolution_file
                default_systematics_options['jer']['scaleFactorFile'] = self.__jer_scalefactor_file

            systematics = {}
            for syst in self.__systematics:
                user_systematics_options = self.__systematicsOptions[syst] if syst in self.__systematicsOptions else {}
                systematics[syst] = copy.deepcopy(default_systematics_options[syst])
                systematics[syst].update(user_systematics_options)

            print("")
            Systematics.doSystematics(self, systematics)


        # Add the framework to the path as the last element
        self.path += cms.Sequence(self.process.framework)
        self.process.p = self.path

        if self.verbose:
            print("")
            print("Framework configuration done.")
            print("    Producers: %s" % ', '.join(self.producers))
            print("    Analyzers: %s" % ', '.join(self.analyzers))
            print("")

        # Specify scheduling of analyzers and producers
        self.process.framework.analyzers_scheduling = cms.untracked.vstring(self.analyzers)
        self.process.framework.producers_scheduling = cms.untracked.vstring(self.producers)

        return self.process

    @dep(before=("create", "correction"))
    def addAnalyzer(self, name, configuration, index=None):
        """
        Add an analyzer in the framework configuration with a given name and configuration
        """

        if name in self.analyzers:
            raise Exception('An analyzer named %r is already added to the configuration' % name)

        index = index if index is not None else len(self.analyzers)

        self.analyzers.insert(index, name)
        setattr(self.process.framework.analyzers, name, configuration)

    @dep(before=("create", "correction"))
    def addProducer(self, name, configuration, index=None):
        """
        Add a producer in the framework configuration with a given name and configuration
        """

        if name in self.producers:
            raise Exception('A producer named %r is already added to the configuration' % name)

        index = index if index is not None else len(self.producers)

        self.producers.insert(index, name)
        setattr(self.process.framework.producers, name, configuration)

    @dep(before=("create", "correction"))
    def getProducer(self, name):
        """
        Return a producer
        """

        if not name in self.producers:
            raise Exception('No producer named %r found in the configuration' % name)

        producer = getattr(self.process.framework.producers, name)
        return producer

    @dep(before=("create", "correction"))
    def removeAnalyzer(self, name):
        """
        Remove an analyzer from the framework configuration
        """

        if not name in self.analyzers:
            raise Exception('Analyzer %r is not present in the framework configuration' % name)

        self.analyzers.remove(name)
        delattr(self.process.framework.analyzers, name)

    @dep(before=("create", "correction"))
    def removeProducer(self, name):
        """
        Remove a producer from the framework configuration
        """

        if not name in self.producers:
            raise Exception('Producer %r is not present in the framework configuration' % name)

        self.producers.remove(name)
        delattr(self.process.framework.producers, name)

    @dep(before=("create", "correction"))
    def getAnalyzerIndex(self, name):
        """
        Return the current index of an analyzer
        """

        if not name in self.analyzers:
            raise Exception('Analyzer %r is not present in the framework configuration' % name)

        return self.analyzers.index(name)

    @dep(before=("create", "correction"))
    def getProducerIndex(self, name):
        """
        Return the current index of a producer
        """

        if not name in self.producers:
            raise Exception('Producer %r is not present in the framework configuration' % name)

        return self.producers.index(name)

    @dep(before=("create", "correction"))
    def useJECDatabase(self, database):
        """
        JEC factors will be retrieved from the database instead of the Global Tag
        """

        # Read the JEC from a database
        from cp3_llbb.Framework.Tools import load_jec_from_db
        algo_sizes = {'ak': [4, 8]}
        jet_types = ['pf', 'pfchs', 'puppi']
        jet_algos = []
        for k, v in algo_sizes.iteritems():
            for s in v:
                for j in jet_types:
                    jet_algos.append(str(k.upper() + str(s) + j.upper().replace("CHS", "chs").replace("PUPPI", "PFPuppi")))

        load_jec_from_db(self.process, database, jet_algos)

    @dep(before=("create", "jec", "jer"), performs=("correction", "jec"))
    def redoJEC(self, JECDatabase=None, addBtagDiscriminators=None):
        """
        Redo the Jet Energy Corrections for the default AK4 jet collection,
        and optionally add some b-tags (which implies redoing the JEC, since
        b-tagging needs to be run on uncalibrated jets).

        FIXME: Some love is needed for AK8 jets
        """

        if self.verbose:
            print("")
            print("Redoing Jet Energy Corrections{}".format("" if addBtagDiscriminators is None else ", and adding b-tag classifiers {}".format(", ".join(addBtagDiscriminators))))

        if JECDatabase:
            # Read the JEC from a database
            self.useJECDatabase(JECDatabase)

        from cp3_llbb.Framework.Tools import recorrect_jets, recorrect_met
        jet_collection = recorrect_jets(self.process, self.isData, 'AK4PFchs', self.__miniaod_jet_collection, addBtagDiscriminators=addBtagDiscriminators)
        met_collection = recorrect_met(self.process, self.isData, self.__miniaod_met_collection, jet_collection)

        # Fat jets
        fat_jet_collection = recorrect_jets(self.process, self.isData, 'AK8PFchs', self.__miniaod_fat_jet_collection, addBtagDiscriminators=addBtagDiscriminators)

        # Look for producers using the default jet and met collections
        for producer in self.producers:
            p = getattr(self.process.framework.producers, producer)
            change_input_tags_and_strings(p, self.__miniaod_jet_collection, jet_collection, 'producers.' + producer, '    ')
            change_input_tags_and_strings(p, self.__miniaod_fat_jet_collection, fat_jet_collection, 'producers.' + producer, '    ')
            change_input_tags_and_strings(p, self.__miniaod_met_collection, met_collection, 'producers.' + producer, '    ')

            if p.type == 'met':
                p.parameters.slimmed = cms.untracked.bool(False)

        # Change the default collections to the newly created
        self.__miniaod_jet_collection = jet_collection
        self.__miniaod_unsmeared_jet_collection = jet_collection
        self.__miniaod_fat_jet_collection = fat_jet_collection
        self.__miniaod_met_collection = met_collection

        if self.verbose:
            print("New jets and MET collections: %r, %r and %r" % (jet_collection, fat_jet_collection, met_collection))

        print("")

    @dep(before=("create", "jer"), performs=("correction", "jer"))
    def smearJets(self, resolutionFile=None, scaleFactorFile=None):
        """
        Smear the jets
        """

        if self.isData:
            return

        useTxtFiles = resolutionFile and scaleFactorFile

        if self.verbose:
            print("")
            print("Smearing jets...")
            if useTxtFiles:
                print("  -> {}".format(resolutionFile))
                print("  -> {}".format(scaleFactorFile))

        self.process.slimmedJetsSmeared = cms.EDProducer('SmearedPATJetProducer',
                    src = cms.InputTag(self.__miniaod_jet_collection),
                    enabled = cms.bool(True),
                    rho = cms.InputTag("fixedGridRhoFastjetAll"),
                    algo = cms.string('AK4PFchs'),
                    algopt = cms.string('AK4PFchs_pt'),

                    genJets = cms.InputTag(self.__miniaod_gen_jet_collection),
                    dRMax = cms.double(0.2),
                    dPtMaxFactor = cms.double(3),

                    variation = cms.int32(0)
                )

        if useTxtFiles:
            del self.process.slimmedJetsSmeared.algo
            del self.process.slimmedJetsSmeared.algopt
            self.process.slimmedJetsSmeared.resolutionFile = cms.FileInPath(resolutionFile)
            self.process.slimmedJetsSmeared.scaleFactorFile = cms.FileInPath(scaleFactorFile)
            self.__jer_resolution_file = resolutionFile
            self.__jer_scalefactor_file = scaleFactorFile

        self.process.shiftedMETCorrModuleForSmearedJets = cms.EDProducer('ShiftedParticleMETcorrInputProducer',
                srcOriginal = cms.InputTag(self.__miniaod_jet_collection),
                srcShifted = cms.InputTag('slimmedJetsSmeared')
                )

        self.process.slimmedMETsSmeared = cms.EDProducer('CorrectedPATMETProducer',
                src = cms.InputTag(self.__miniaod_met_collection),
                srcCorrections = cms.VInputTag(cms.InputTag('shiftedMETCorrModuleForSmearedJets'))
                )

        # Look for producers using the default jet and met collections
        for producer in self.producers:
            p = getattr(self.process.framework.producers, producer)
            change_input_tags_and_strings(p, self.__miniaod_jet_collection, 'slimmedJetsSmeared', 'producers.' + producer, '    ')
            change_input_tags_and_strings(p, self.__miniaod_met_collection, 'slimmedMETsSmeared', 'producers.' + producer, '    ')

            if p.type == 'met':
                p.parameters.slimmed = cms.untracked.bool(False)

        self.__miniaod_jet_collection = 'slimmedJetsSmeared'
        self.__miniaod_met_collection = 'slimmedMETsSmeared'

        if self.verbose:
            print("New jets and MET collections: %r and %r" % (self.__miniaod_jet_collection, self.__miniaod_met_collection))

    @dep(before=("create", "muonScale"), performs=("correction", "muonScale"))
    def applyMuonCorrection(self, type):
        """
        Apply correction to muon

        Parameters:
            type: either "rochester" or "kamuca". Define the type of correction to apply
        """

        supported = ['kamuca', 'rochester']

        if not type in supported:
            raise Exception("Unsupported muon correction. Can only be one of %s" % supported)

        if self.verbose:
            print("")
            print("Applying %s correction to muons..." % type.capitalize())

        if type == "kamuca":
            self.process.slimmedMuonsCorrected = cms.EDProducer('KaMuCaCorrectedPATMuonProducer',
                        src = cms.InputTag(self.__miniaod_muon_collection),
                        enabled = cms.bool(True),
                        tag = cms.string(self.__kamuca_tag)
                    )
        elif type == "rochester":
            self.process.slimmedMuonsCorrected = cms.EDProducer('RochesterCorrectedPATMuonProducer',
                        src = cms.InputTag(self.__miniaod_muon_collection),
                        enabled = cms.bool(True),
                        input = cms.FileInPath(self.__rochester_input)
                    )

        # Look for producers using the default muon input
        for producer in self.producers:
            p = getattr(self.process.framework.producers, producer)
            change_input_tags_and_strings(p, self.__miniaod_muon_collection, 'slimmedMuonsCorrected', 'producers.' + producer, '    ')

        self.__miniaod_muon_collection = 'slimmedMuonsCorrected'

        if self.verbose:
            print("New muons collection: %r" % (self.__miniaod_muon_collection))

    @dep(before=("create", "electronRegression"), performs=("correction", "electronRegression"))
    def applyElectronRegression(self):
        """
        Apply electron regression from
            https://twiki.cern.ch/twiki/bin/view/CMS/EGMRegression
        """

        if self.verbose:
            print("")
            print("Applying electron regression...")

        # Read corrections for database
        from EgammaAnalysis.ElectronTools.regressionWeights_cfi import regressionWeights
        regressionWeights(self.process)

        self.process.load('EgammaAnalysis.ElectronTools.regressionApplication_cff')

        # Rename the collection
        self.process.slimmedElectronsWithRegression = self.process.slimmedElectrons.clone()

        # Look for producers using the default electron input
        for producer in self.producers:
            p = getattr(self.process.framework.producers, producer)
            change_input_tags_and_strings(p, self.__miniaod_electron_collection, 'slimmedElectronsWithRegression', 'producers.' + producer, '    ')

        self.__miniaod_electron_collection = 'slimmedElectronsWithRegression'

        if self.verbose:
            print("New electrons collection: %r" % (self.__miniaod_electron_collection))

    @dep(before=("create", "electronSmearing"), after="electronRegression", performs=("electronSmearing", "correction"))
    def applyElectronSmearing(self):
        """
        Apply electron smearing from
            https://twiki.cern.ch/twiki/bin/view/CMS/EGMSmearer
        """

        if self.verbose:
            print("")
            print("Applying electron smearing...")

        from EgammaAnalysis.ElectronTools.calibratedPatElectronsRun2_cfi import calibratedPatElectrons
        from EgammaAnalysis.ElectronTools.calibrationTablesRun2 import files

        # FIXME: Add a preselection on electron to prevent a crash in the producer
        # Remove when it's no longer needed (see twiki)
        self.process.selectedElectrons = cms.EDFilter("PATElectronSelector",
                src = cms.InputTag(self.__miniaod_electron_collection),
                cut = cms.string("pt > 5 && abs(superCluster.eta) < 2.5")
                )

        self.process.slimmedElectronsSmeared = calibratedPatElectrons.clone(
                electrons = "selectedElectrons",
                isMC = not self.isData,
                correctionFile = files['Moriond17_23Jan']
                )

        self.process.load('Configuration.StandardSequences.Services_cff')
        self.process.RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
                slimmedElectronsSmeared = cms.PSet(
                    initialSeed = cms.untracked.uint32(42),
                    engineName = cms.untracked.string('TRandom3')
                    )
                )

        # Look for producers using the default electron input
        for producer in self.producers:
            p = getattr(self.process.framework.producers, producer)
            change_input_tags_and_strings(p, self.__miniaod_electron_collection, 'slimmedElectronsSmeared', 'producers.' + producer, '    ')

        self.__miniaod_electron_collection = 'slimmedElectronsSmeared'

        if self.verbose:
            print("New electrons collection: %r" % (self.__miniaod_electron_collection))

    @dep(before="create")
    def doSystematics(self, systematics, **kwargs):
        """
        Enable systematics
        """

        if self.isData:
            return

        self.__systematics = systematics
        self.__systematicsOptions = kwargs


    #
    # Private methods
    #

    @dep(before="create")
    def configureElectronId_(self):

        with StdStreamSilenter():
          self.process.load("RecoEgamma.Phase2InterimID.phase2EgammaPAT_cff")
          self.path += cms.Sequence(self.process.phase2Egamma)

    def _configureFramework(self):

        from cp3_llbb.Framework import EventProducer
        from cp3_llbb.Framework import GenParticlesProducer
        from cp3_llbb.Framework import HLTProducer
        from cp3_llbb.Framework import JetsProducer
        from cp3_llbb.Framework import FatJetsProducer
        from cp3_llbb.Framework import METProducer
        from cp3_llbb.Framework import MuonsProducer
        from cp3_llbb.Framework import ElectronsProducer
        from cp3_llbb.Framework import VerticesProducer

        self.process.framework = cms.EDProducer("ExTreeMaker",
                output = cms.string(self.output_filename),
                filters = cms.PSet(),
                producers = cms.PSet(),
                analyzers = cms.PSet()
                )

        self.addProducer('event', copy.deepcopy(EventProducer.default_configuration))
        self.addProducer('hlt', copy.deepcopy(HLTProducer.default_configuration))
        self.addProducer('jets', copy.deepcopy(JetsProducer.default_configuration))
        self.addProducer('fat_jets', copy.deepcopy(FatJetsProducer.default_configuration))
        self.addProducer('met', copy.deepcopy(METProducer.default_configuration))
        self.addProducer('muons', copy.deepcopy(MuonsProducer.default_configuration))
        self.addProducer('electrons', copy.deepcopy(ElectronsProducer.default_configuration))
        self.addProducer('vertices', copy.deepcopy(VerticesProducer.default_configuration))

        if not self.isData:
            self.addProducer('gen_particles', copy.deepcopy(GenParticlesProducer.default_configuration), 0)
        else:
            # MET Filters
            from cp3_llbb.Framework import METFilter
            self.process.framework.filters.met = copy.deepcopy(METFilter.default_configuration)
