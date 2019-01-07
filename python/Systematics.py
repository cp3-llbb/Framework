import copy

import FWCore.ParameterSet.Config as cms

from cp3_llbb.Framework.Tools import change_input_tags_and_strings, module_has_string

framework_skipDepChecks = ("correction",)

class Systematics(object):
    def __init__(self, name, framework):
        self.name = name
        self.framework = framework
        self.process = framework.process
        self.verbosity = True

    def getPrettyName(self, postfix):
        return self.name.upper() + postfix#.capitalize()

    def formatModuleName(self, collection, postfix):
        return collection + self.getPrettyName(postfix)

    def formatModuleVariableName(self, collection, postfix):
        if collection[-1] == '_':
            collection = collection[:-1]
        return collection + "_" + self.getPrettyName(postfix).lower()

class JetsSystematics(Systematics):
    def __init__(self, name, framework, jetCollection, metCollection):
        super(JetsSystematics, self).__init__(name, framework)
        self.jetCollection = jetCollection
        self.metCollection = metCollection

    def getJetSystematicsProducers_(self, shift):
        raise NotImplementedException()

    def addShiftedMETProducer_(self, newJetCollection, postfix):
        """
        Create a shifted MET collection using the difference between
        jetCollection and newJetCollection
        """
        shiftedMETCorrModule = cms.EDProducer('ShiftedParticleMETcorrInputProducer',
                srcOriginal = cms.InputTag(self.jetCollection),
                srcShifted = cms.InputTag(newJetCollection)
                )

        shiftedMETCorrModuleName = self.formatModuleName('shiftedPatMETCorr', postfix)
        setattr(self.process, shiftedMETCorrModuleName, shiftedMETCorrModule)

        # Create MET module, starting from original MET and applying corrections
        # computed from new jets collection
        shiftedMETModule = cms.EDProducer('CorrectedPATMETProducer',
                src = cms.InputTag(self.metCollection),
                srcCorrections = cms.VInputTag(cms.InputTag(shiftedMETCorrModuleName))
                )

        shiftedMETModuleName = self.formatModuleName(self.metCollection, postfix)
        setattr(self.process, shiftedMETModuleName, shiftedMETModule)

        self.framework.path.associate(cms.Task(*(getattr(self.process, modName) for modName in (shiftedMETCorrModuleName, shiftedMETModuleName))))

        return shiftedMETModule


    def addJetsProducer(self, inputCollection, postfix):
        producerName = self.formatModuleVariableName('jets', postfix)
    
        cfg = copy.deepcopy(self.process.framework.producers.jets)
        
        cfg.prefix = self.formatModuleVariableName(cfg.prefix.value(), postfix) + '_'
        cfg.parameters.jets = cms.untracked.InputTag(inputCollection)
        cfg.parameters.systematics = cms.untracked.bool(True)
       
        index = self.framework.getProducerIndex('jets', skipDepsCheck=framework_skipDepChecks)
        self.framework.addProducer(producerName, cfg, index + 1, skipDepsCheck=framework_skipDepChecks)

    def addMETProducer(self, inputCollection, postfix):
        producerName = self.formatModuleVariableName('met', postfix)
    
        cfg = copy.deepcopy(self.process.framework.producers.met)
        
        cfg.prefix = self.formatModuleVariableName(cfg.prefix.value(), postfix) + '_'
        cfg.parameters.met = cms.untracked.InputTag(inputCollection)
        cfg.parameters.slimmed = cms.untracked.bool(False)
        cfg.parameters.systematics = cms.untracked.bool(True)
       
        index = self.framework.getProducerIndex('met', skipDepsCheck=framework_skipDepChecks)
        self.framework.addProducer(producerName, cfg, index + 1, skipDepsCheck=framework_skipDepChecks)

    def run(self):
        upModules = self.getJetSystematicsProducers_(+1)
        downModules = self.getJetSystematicsProducers_(-1)

        for i in range(len(upModules)):

            upModule, upModulePostfix = upModules[i]
            downModule, downModulePostfix = downModules[i]

            upModulePostfix += "Up"
            downModulePostfix += "Down"

            upModuleInputTag = None
            downModuleInputTag = None

            if type(upModule) is tuple:
                assert(len(upModule[0].label()) > 0)
                upModuleInputTag = cms.InputTag(upModule[0].label(), upModule[1])
                downModuleInputTag = cms.InputTag(downModule[0].label(), downModule[1])
            else:
                upModName = self.formatModuleName(self.jetCollection, upModulePostfix)
                setattr(self.process, upModName, upModule)
                downModName = self.formatModuleName(self.jetCollection, downModulePostfix)
                setattr(self.process, downModName, downModule)
                upModuleInputTag = cms.InputTag(upModule.label())
                downModuleInputTag = cms.InputTag(downModule.label())

                self.framework.path.associate(cms.Task(*(getattr(self.process, modName) for modName in (upModName, downModName))))

            # Shifted MET
            upMetModule = self.addShiftedMETProducer_(upModuleInputTag.value(), upModulePostfix)
            downMetModule = self.addShiftedMETProducer_(downModuleInputTag.value(), downModulePostfix)

            # Add Framework producers for these new jets collections
            self.addJetsProducer(upModuleInputTag.value(), upModulePostfix)
            self.addJetsProducer(downModuleInputTag.value(), downModulePostfix)

            self.addMETProducer(upMetModule.label(), upModulePostfix)
            self.addMETProducer(downMetModule.label(), downModulePostfix)

            upJetProducer = self.formatModuleVariableName('jets', upModulePostfix)
            downJetProducer = self.formatModuleVariableName('jets', downModulePostfix)
            upMetProducer = self.formatModuleVariableName('met', upModulePostfix)
            downMetProducer = self.formatModuleVariableName('met', downModulePostfix)

            if self.framework.verbose:
                print("    Adding new CMSSW module %r as %r framework producer") % (upModuleInputTag.value(), upJetProducer)
                print("    Adding new CMSSW module %r as %r framework producer") % (downModuleInputTag.value(), downJetProducer)
                print("    Adding new CMSSW module %r as %r framework producer") % (upMetModule.label(), upMetProducer)
                print("    Adding new CMSSW module %r as %r framework producer") % (downMetModule.label(), downMetProducer)

            analyzers = []
            for analyzer in self.framework.analyzers:
                module = getattr(self.process.framework.analyzers, analyzer)
                if module_has_string(module, 'jets') or module_has_string(module, 'met'):
                    analyzers += [(analyzer, module)]

            for analyzer, cfg in analyzers:
                upCfg = copy.deepcopy(cfg)
                downCfg = copy.deepcopy(cfg)

                upCfg.parameters.systematics = cms.untracked.bool(True)
                downCfg.parameters.systematics = cms.untracked.bool(True)

                upAnalyzer = self.formatModuleVariableName(analyzer, upModulePostfix)
                downAnalyzer = self.formatModuleVariableName(analyzer, downModulePostfix)

                if self.framework.verbose:
                    print "    Cloning %r analyzer to %r to run over new collections" % (analyzer, upAnalyzer)
                padding = ' ' * 8
                change_input_tags_and_strings(upCfg, 'jets', upJetProducer, analyzer, padding)
                change_input_tags_and_strings(upCfg, 'met', upMetProducer, analyzer, padding)

                if self.framework.verbose:
                    print "    Cloning %r analyzer to %r to run over new collections" % (analyzer, downAnalyzer)
                change_input_tags_and_strings(downCfg, 'jets', downJetProducer, analyzer, padding)
                change_input_tags_and_strings(downCfg, 'met', downMetProducer, analyzer, padding)

                upCfg.prefix = self.formatModuleVariableName(upCfg.prefix.value(), upModulePostfix) + '_'
                downCfg.prefix = self.formatModuleVariableName(downCfg.prefix.value(), downModulePostfix) + '_'

                index = self.framework.getAnalyzerIndex(analyzer, skipDepsCheck=framework_skipDepChecks)
                self.framework.addAnalyzer(upAnalyzer, upCfg, index + 1, skipDepsCheck=framework_skipDepChecks)
                self.framework.addAnalyzer(downAnalyzer, downCfg, index + 2, skipDepsCheck=framework_skipDepChecks)


class JECSystematics(JetsSystematics):
    def __init__(self, name, framework, jetCollection, metCollection, uncertaintiesFile, splitBySources=False):
        super(JECSystematics, self).__init__(name, framework, jetCollection, metCollection)
        self.uncertaintiesFile = uncertaintiesFile
        self.splitBySources = splitBySources

        if self.splitBySources and not self.uncertaintiesFile:
            raise Exception("You asked to split the JEC uncertainties, but you did not provide a text files")

    def getJetSystematicsProducers_(self, shift):

        modules = []

        # Total uncertainty
        module = cms.EDProducer('ShiftedPATJetProducerWithSources',
                    enabled = cms.bool(True),
                    src = cms.InputTag(self.jetCollection),
                    shiftBy = cms.double(shift),
                    splitBySources = cms.bool(self.splitBySources)
                )

        if self.uncertaintiesFile:
            module.sources = cms.FileInPath(self.uncertaintiesFile)

        modules.append((module, ""))

        if self.splitBySources:
            sources = JECSystematics.getJECSources()
            for source in sources:
                modules.append(((module, source), source))

        return modules

    @staticmethod
    def getJECSources():
        return [
                "AbsoluteFlavMap",
                "AbsoluteMPFBias",
                "AbsoluteScale",
                "AbsoluteStat",
                "FlavorQCD",
                "Fragmentation",
                "PileUpDataMC",
                "PileUpPtBB",
                "PileUpPtEC1",
                "PileUpPtEC2",
                "PileUpPtHF",
                "PileUpPtRef",
                "RelativeBal",
                "RelativeFSR",
                "RelativeJEREC1",
                "RelativeJEREC2",
                "RelativeJERHF",
                "RelativePtBB",
                "RelativePtEC1",
                "RelativePtEC2",
                "RelativePtHF",
                "RelativeStatEC",
                "RelativeStatFSR",
                "RelativeStatHF",
                "SinglePionECAL",
                "SinglePionHCAL",
                "TimePtEta"
                ]

class JERSystematics(JetsSystematics):
    def __init__(self, name, framework, jetCollection, metCollection, genJetCollection, resolutionFile=None, scaleFactorFile=None):
        super(JERSystematics, self).__init__(name, framework, jetCollection, metCollection)
        self.genJetCollection = genJetCollection
        self.resolutionFile = resolutionFile
        self.scaleFactorFile = scaleFactorFile

    def getJetSystematicsProducers_(self, shift):

        module = cms.EDProducer('SmearedPATJetProducer',
                    src = cms.InputTag(self.jetCollection),
                    enabled = cms.bool(True),
                    rho = cms.InputTag("fixedGridRhoFastjetAll"),
                    algo = cms.string('AK4PFchs'),
                    algopt = cms.string('AK4PFchs_pt'),

                    genJets = cms.InputTag(self.genJetCollection),
                    dRMax = cms.double(0.2),
                    dPtMaxFactor = cms.double(3),

                    variation = cms.int32(shift)
                )

        if self.resolutionFile and self.scaleFactorFile:
            del module.algo
            del module.algopt
            module.resolutionFile = cms.FileInPath(self.resolutionFile)
            module.scaleFactorFile = cms.FileInPath(self.scaleFactorFile)

        return [(module, "")]


handlers = {'jec': JECSystematics, 'jer': JERSystematics}

def doSystematics(framework, systematics):

    for syst, options in systematics.items():
        if not syst in handlers:
            print("Warning: trying to compute %r systematics, but no handler is defined for this systematics")
            continue

        if framework.verbose:
            print 'Configuring framework for %r systematics' % syst

        cls = handlers[syst]
        instance = cls(syst, framework, **options)
        instance.run()

        if framework.verbose:
            print ''
