import copy

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

from cp3_llbb.Framework.Tools import change_process_name, change_input_tags_and_strings, StdStreamSilenter
from cp3_llbb.Framework import Tools
import cp3_llbb.Framework.Systematics as Systematics

class Framework(object):

    def __init__(self, isData, era, **kwargs):
        self.__created = False
        self.__systematics = []
        self.__systematicsOptions = {}
        self.__jec_done = False
        self.__jer_done = False

        # Default configuration
        self.__miniaod_jet_collection = 'slimmedJets'
        self.__miniaod_fat_jet_collection = 'slimmedJetsAK8'
        self.__miniaod_gen_jet_collection = 'slimmedGenJets'
        self.__miniaod_met_collection = 'slimmedMETs'

        self.isData = isData
        self.era = era
        self.processName = kwargs['processName'] if 'processName' in kwargs else 'PAT'
        self.globalTag = kwargs['globalTag'] if 'globalTag' in kwargs else None
        self.verbose = kwargs['verbose'] if 'verbose' in kwargs else True
        self.output_filename = 'output_data.root' if isData else 'output_mc.root'
        self.producers = []
        self.analyzers = []

        Tools.verbosity = self.verbose

        self.parseCommandLine_()

        if self.globalTag is None:
            raise Exception("No global tag specified. Use the 'globalTag' command line argument to set one.\n\tcmsRun <configuration> globalTag=<global_tag>")

        if self.era is None:
            raise Exception("No era specified. Use the 'era' command line argument to set one.\n\tcmsRun <configuration> era=[25ns|50ns]")

        if self.verbose:
            print("")
            print("CP3 llbb framework:")
            print("    - Running over %s" % ("data" if self.isData else "simulation"))
            print("    - global tag: %s" % self.globalTag)
            print("    - era: %s" % ("25ns" if self.era == eras.Run2_25ns else "50ns"))
            print("")

        # Create CMSSW process and configure it with sane default values

        process = cms.Process("ETM", era)
        self.process = process
        self.path = cms.Path()

        process.options = cms.untracked.PSet(
                wantSummary = cms.untracked.bool(False),
                allowUnscheduled = cms.untracked.bool(True)
                )

        # Create an empty PSet for communication with GridIn
        process.gridin = cms.PSet(
                input_files = cms.vstring()
                )

        process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
        process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
        process.load('Configuration.StandardSequences.MagneticField_38T_cff')
        process.load('FWCore.MessageLogger.MessageLogger_cfi')


        process.GlobalTag.globaltag = self.globalTag

        process.MessageLogger.cerr.FwkReport.reportEvery = 1000
        process.MessageLogger.suppressWarning = cms.untracked.vstring('framework')

        process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(20))
        process.source = cms.Source("PoolSource")

        self.configureElectronId_()
        self.configureFramework_()

    def create(self):
        """
        Terminate the process configuration and return the final CMSSW process
        """

        if self.__created:
            return self.process

        # Change process name if needed
        if self.processName is not None and self.processName != 'PAT':
            if self.verbose:
                print("")
                print("Changing process name from %r to %r..." % ('PAT', self.processName))
            change_process_name(self.process.framework, 'PAT', self.processName)

        if len(self.__systematics) > 0:
            if self.verbose:
                print("")

            default_systematics_options = {
                    'jec': {'jetCollection': self.__miniaod_jet_collection,
                        'metCollection': self.__miniaod_met_collection,
                        'uncertaintiesFile': None},
                    'jer': {'jetCollection': self.__miniaod_jet_collection,
                        'metCollection': self.__miniaod_met_collection,
                        'genJetCollection': self.__miniaod_gen_jet_collection}
                    }

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

        self.__created = True
        return self.process

    def addAnalyzer(self, name, configuration, index=None):
        """
        Add an analyzer in the framework configuration with a given name and configuration
        """

        self.ensureNotCreated()

        if name in self.analyzers:
            raise Exception('An analyzer named %r is already added to the configuration' % name)

        index = index if index is not None else len(self.analyzers)

        self.analyzers.insert(index, name)
        setattr(self.process.framework.analyzers, name, configuration)

    def addProducer(self, name, configuration, index=None):
        """
        Add a producer in the framework configuration with a given name and configuration
        """

        self.ensureNotCreated()

        if name in self.producers:
            raise Exception('A producer named %r is already added to the configuration' % name)

        index = index if index is not None else len(self.producers)

        self.producers.insert(index, name)
        setattr(self.process.framework.producers, name, configuration)

    def getProducer(self, name):
        """
        Return a producer
        """

        self.ensureNotCreated()

        if not name in self.producers:
            raise Exception('No producer named %r found in the configuration' % name)

        producer = getattr(self.process.framework.producers, name)
        return producer

    def removeAnalyzer(self, name):
        """
        Remove an analyzer from the framework configuration
        """

        self.ensureNotCreated()

        if not name in self.analyzers:
            raise Exception('Analyzer %r is not present in the framework configuration' % name)

        self.analyzers.remove(name)
        delattr(self.process.framework.analyzers, name)

    def removeProducer(self, name):
        """
        Remove a producer from the framework configuration
        """

        self.ensureNotCreated()

        if not name in self.producers:
            raise Exception('Producer %r is not present in the framework configuration' % name)

        self.producers.remove(name)
        delattr(self.process.framework.producers, name)

    def getAnalyzerIndex(self, name):
        """
        Return the current index of an analyzer
        """

        self.ensureNotCreated()

        if not name in self.analyzers:
            raise Exception('Analyzer %r is not present in the framework configuration' % name)

        return self.analyzers.index(name)

    def getProducerIndex(self, name):
        """
        Return the current index of a producer
        """

        self.ensureNotCreated()

        if not name in self.producers:
            raise Exception('Producer %r is not present in the framework configuration' % name)

        return self.producers.index(name)

    def useJECDatabase(self, database):
        """
        JEC factors will be retrieved from the database instead of the Global Tag
        """

        self.ensureNotCreated()

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


    def redoJEC(self, JECDatabase=None):
        """
        Redo the Jet Energy Corrections for the default AK4 jet collection
        FIXME: Some love is needed for AK8 jets
        """

        self.ensureNotCreated()

        if self.__jer_done:
            raise Exception("You must smear the jets after doing the Jet Energy Corrections. Please call 'redoJEC' before 'smearJets'")

        if self.verbose:
            print("")
            print("Redoing Jet Energy Corrections")

        if JECDatabase:
            # Read the JEC from a database
            self.useJECDatabase(JECDatabase)

        from cp3_llbb.Framework.Tools import recorrect_jets, recorrect_met
        jet_collection = recorrect_jets(self.process, self.isData, 'AK4PFchs', self.__miniaod_jet_collection)
        met_collection = recorrect_met(self.process, self.isData, self.__miniaod_met_collection, jet_collection)

        # Fat jets
        fat_jet_collection = recorrect_jets(self.process, self.isData, 'AK8PFchs', self.__miniaod_fat_jet_collection)

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
        self.__miniaod_fat_jet_collection = fat_jet_collection
        self.__miniaod_met_collection = met_collection

        if self.verbose:
            print("New jets and MET collections: %r, %r and %r" % (jet_collection, fat_jet_collection, met_collection))

        print("")

        self.__jec_done = True

    def smearJets(self):
        """
        Smear the jets
        """

        self.ensureNotCreated()

        if self.isData:
            return

        if self.verbose:
            print("")
            print("Smearing jets...")

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

        self.__jer_done = True

    def doSystematics(self, systematics, **kwargs):
        """
        Enable systematics
        """

        self.ensureNotCreated()

        if self.isData:
            return

        self.__systematics = systematics
        self.__systematicsOptions = kwargs


    #
    # Private methods
    #

    def ensureNotCreated(self):
        if self.__created:
            raise RuntimeError("The framework configuration is frozen. Framework.create() must be the last function called.")

    def parseCommandLine_(self):
        """
        Parse the command line for options. Supported options are 'globalTag', 'era' and 'process'.

        Used mainly by GridIn and crab to override user configuration of the framework with correct values
        for a given dataset.
        """

        self.ensureNotCreated()

        import sys

        parseCommandLine = False
        for argv in sys.argv:
            if 'runOnData' in argv or 'globalTag' in argv or 'era' in argv or 'process' in argv:
                parseCommandLine = True
                break

        if parseCommandLine:
            from FWCore.ParameterSet.VarParsing import VarParsing
            options = VarParsing()
            options.register('runOnData',
                    -1,
                    VarParsing.multiplicity.singleton,
                    VarParsing.varType.int,
                    'If running over MC (0) or data (1)')

            options.register('globalTag',
                    '',
                    VarParsing.multiplicity.singleton,
                    VarParsing.varType.string,
                    'The globaltag to use')

            options.register('era',
                    '',
                    VarParsing.multiplicity.singleton,
                    VarParsing.varType.string,
                    'Era of the dataset')

            options.register('process',
                    '',
                    VarParsing.multiplicity.singleton,
                    VarParsing.varType.string,
                    'Process name of the MiniAOD production.')

            options.parseArguments()

            if options.runOnData != -1:
                runOnData = options.runOnData == 1
                if self.isData != runOnData:
                    raise Exception("Inconsistency between the command line argument 'runOnData' and the first argument of the Framework constructor.")

            if options.globalTag:
                self.globalTag = options.globalTag

            if options.era:
                assert options.era == '25ns' or options.era == '50ns'
                if options.era == '25ns':
                    self.era = eras.Run2_25ns
                else:
                    self.era = eras.Run2_50ns

            if options.process:
                self.processName = options.process

    def configureElectronId_(self):

        self.ensureNotCreated()

        with StdStreamSilenter():
            from PhysicsTools.SelectorUtils.tools.vid_id_tools import DataFormat, switchOnVIDElectronIdProducer, setupAllVIDIdsInModule, setupVIDElectronSelection
            switchOnVIDElectronIdProducer(self.process, DataFormat.MiniAOD)

            id_modules = [
                    'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff',
                    ]

            for mod in id_modules:
                setupAllVIDIdsInModule(self.process, mod, setupVIDElectronSelection)

    def configureFramework_(self):

        self.ensureNotCreated()

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

        if self.era == eras.Run2_25ns:
            self.process.framework.producers.electrons.parameters.ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt')
        else:
            self.process.framework.producers.electrons.parameters.ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_50ns.txt')

        if not self.isData:
            self.addProducer('gen_particles', copy.deepcopy(GenParticlesProducer.default_configuration), 0)
            self.getProducer('muons').parameters.inputTagKaMuCa = self.getProducer('muons').parameters.inputTagKaMuCaMC
        else:
            # MET Filters
            from cp3_llbb.Framework import METFilter
            self.process.framework.filters.met = copy.deepcopy(METFilter.default_configuration)
            self.getProducer('muons').parameters.inputTagKaMuCa = self.getProducer('muons').parameters.inputTagKaMuCaData
