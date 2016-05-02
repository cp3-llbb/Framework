import copy

import FWCore.ParameterSet.Config as cms

from cp3_llbb.Framework.Tools import change_input_tags_and_strings, module_has_string

class Systematics(object):
    def __init__(self, name, framework):
        self.name = name
        self.framework = framework
        self.process = framework.process
        self.verbosity = True

    def getPrettyName(self, postfix):
        return self.name.upper() + postfix.capitalize()

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

    def getJetSystematicsProducer_(self, shift):
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

        return shiftedMETModule


    def addJetsProducer(self, inputCollection, postfix):
        producerName = self.formatModuleVariableName('jets', postfix)
    
        cfg = copy.deepcopy(self.process.framework.producers.jets)
        
        cfg.prefix = self.formatModuleVariableName(cfg.prefix.value(), postfix) + '_'
        cfg.parameters.jets = cms.untracked.InputTag(inputCollection)
       
        index = self.framework.getProducerIndex('jets')
        self.framework.addProducer(producerName, cfg, index + 1)

    def addMETProducer(self, inputCollection, postfix):
        producerName = self.formatModuleVariableName('met', postfix)
    
        cfg = copy.deepcopy(self.process.framework.producers.met)
        
        cfg.prefix = self.formatModuleVariableName(cfg.prefix.value(), postfix) + '_'
        cfg.parameters.met = cms.untracked.InputTag(inputCollection)
        cfg.parameters.slimmed = cms.untracked.bool(False)
       
        index = self.framework.getProducerIndex('met')
        self.framework.addProducer(producerName, cfg, index + 1)

    def run(self):
        upModule = self.getJetSystematicsProducer_(+1)
        downModule = self.getJetSystematicsProducer_(-1)

        setattr(self.process, self.formatModuleName(self.jetCollection, 'Up'), upModule)
        setattr(self.process, self.formatModuleName(self.jetCollection, 'Down'), downModule)

        # Shifted MET
        upMetModule = self.addShiftedMETProducer_(upModule.label(), 'Up')
        downMetModule = self.addShiftedMETProducer_(downModule.label(), 'Down')

        # Add Framework producers for these new jets collections
        self.addJetsProducer(upModule.label(), 'Up')
        self.addJetsProducer(downModule.label(), 'Down')

        self.addMETProducer(upMetModule.label(), 'Up')
        self.addMETProducer(downMetModule.label(), 'Down')

        upJetProducer = self.formatModuleVariableName('jets', 'Up')
        downJetProducer = self.formatModuleVariableName('jets', 'Down')
        upMetProducer = self.formatModuleVariableName('met', 'Up')
        downMetProducer = self.formatModuleVariableName('met', 'Down')

        if self.framework.verbose:
            print("    Adding new CMSSW module %r as %r framework producer") % (upModule.label(), upJetProducer)
            print("    Adding new CMSSW module %r as %r framework producer") % (downModule.label(), downJetProducer)
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

            upAnalyzer = self.formatModuleVariableName(analyzer, 'Up')
            downAnalyzer = self.formatModuleVariableName(analyzer, 'Down')

            if self.framework.verbose:
                print "    Cloning %r analyzer to %r to run over new collections" % (analyzer, upAnalyzer)
            padding = ' ' * 8
            change_input_tags_and_strings(upCfg, 'jets', upJetProducer, analyzer, padding)
            change_input_tags_and_strings(upCfg, 'met', upMetProducer, analyzer, padding)

            if self.framework.verbose:
                print "    Cloning %r analyzer to %r to run over new collections" % (analyzer, downAnalyzer)
            change_input_tags_and_strings(downCfg, 'jets', downJetProducer, analyzer, padding)
            change_input_tags_and_strings(downCfg, 'met', downMetProducer, analyzer, padding)

            upCfg.prefix = self.formatModuleVariableName(upCfg.prefix.value(), 'Up') + '_'
            downCfg.prefix = self.formatModuleVariableName(downCfg.prefix.value(), 'Down') + '_'

            index = self.framework.getAnalyzerIndex(analyzer)
            self.framework.addAnalyzer(upAnalyzer, upCfg, index + 1)
            self.framework.addAnalyzer(downAnalyzer, downCfg, index + 2)


class JECSystematics(JetsSystematics):
    def __init__(self, name, framework, jetCollection, metCollection, uncertaintiesFile):
        super(JECSystematics, self).__init__(name, framework, jetCollection, metCollection)
        self.uncertaintiesFile = uncertaintiesFile

    def getJetSystematicsProducer_(self, shift):

        module = cms.EDProducer('ShiftedPATJetProducer',
                    src = cms.InputTag(self.jetCollection),
                    addResidualJES = cms.bool(True),
                    jetCorrLabelUpToL3 = cms.InputTag('ak4PFCHSL1FastL2L3Corrector'),
                    jetCorrLabelUpToL3Res = cms.InputTag('ak4PFCHSL1FastL2L3ResidualCorrector'),
                    shiftBy = cms.double(shift)
                )

        tag = 'Total' if self.uncertaintiesFile else 'Uncertainty'
        module.jetCorrUncertaintyTag = cms.string(tag)

        if self.uncertaintiesFile:
            module.jetCorrInputFileName = cms.FileInPath(self.uncertaintiesFile)
        else:
            module.jetCorrPayloadName = cms.string('AK4PFchs')

        return module

class JERSystematics(JetsSystematics):
    def __init__(self, name, framework, jetCollection, metCollection, genJetCollection, resolutionFile, scaleFactorFile):
        super(JERSystematics, self).__init__(name, framework, jetCollection, metCollection)
        self.genJetCollection = genJetCollection
        self.resolutionFile = resolutionFile
        self.scaleFactorFile = scaleFactorFile

    def getJetSystematicsProducer_(self, shift):

        module = cms.EDProducer('SmearedPATJetProducer',
                    src = cms.InputTag(self.jetCollection),
                    enabled = cms.bool(True),
                    rho = cms.InputTag("fixedGridRhoFastjetAll"),
                    resolutionFile = cms.FileInPath(self.resolutionFile),
                    scaleFactorFile = cms.FileInPath(self.scaleFactorFile),

                    genJets = cms.InputTag(self.genJetCollection),
                    dRMax = cms.double(0.2),
                    dPtMaxFactor = cms.double(3),

                    variation = cms.int32(shift)
                )

        return module


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
