import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

def change_input_tag_process_(input_tag, process_name_from, process_name_to):
    if not isinstance(input_tag, cms.InputTag):
        input_tag = cms.untracked.InputTag(input_tag)

    if len(input_tag.getProcessName()) > 0 and input_tag.getProcessName() == process_name_from:
        old_input_tag = input_tag.value()
        input_tag.setProcessName(process_name_to)
        print("Changing input tag from %r to %r" % (old_input_tag, input_tag.value()))

    return input_tag

def change_process_name_(module, process_name_from, process_name_to):
    if isinstance(module, cms._Parameterizable):
        for name in module.parameters_().keys():
            value = getattr(module, name)
            type = value.pythonTypeName()

            if 'VInputTag' in type:
                for (i, tag) in enumerate(value):
                    value[i] = change_input_tag_process_(tag, process_name_from, process_name_to)
            elif 'InputTag' in type:
                change_input_tag_process_(value, process_name_from, process_name_to)

            if isinstance(value, cms._Parameterizable):
                change_process_name_(value, process_name_from, process_name_to)


def create(isData, era, globalTag=None, analyzers=cms.PSet(), redoJEC=False, JECDatabase=None, process_name=None):
    """Create the CMSSW python configuration for the Framework

    Args:
        isData (bool): Set to True if you run over data, or False for simulated samples
        era (Configuration.StandardSequences.Eras.eras): The era of the sample. Used to distringuish between 50ns and 25ns
        globalTag (str): The global tag to use for this workflow. If set to ``None``, a command-line argument named ``globalTag`` must be specified
        analyzers (cms.PSet()): A list of analyzers to run. By default, it's empty, and you can still add one to the list afterwards.
        redoJEC (bool): If True, a new jet collection will be created, starting from MiniAOD jets but with latest JEC, pulled from the global tag.
        JECDatabase (str): If not `None`, then JECs will be read from this database.
        process_name (str): The process name used for the MiniAOD step. Default to 'PAT'.

    Returns:
        The ``process`` object for the CMSSW framework

        You can use the returned object to add / remove producers, analyzers from the default framework configuration.

    Examples:

        import FWCore.ParameterSet.Config as cms
        from Configuration.StandardSequences.Eras import eras

        from cp3_llbb.Framework import Framework

        # Run on 50ns data sample
        process = Framework.create(True, eras.Run2_50ns, '74X_dataRun2_Prompt_v1')

        # Run on 25ns data sample
        process = Framework.create(True, eras.Run2_25ns, '74X_dataRun2_Prompt_v2')

        # For MC 50ns
        process = Framework.create(False, eras.Run2_50ns, 'MCRUN2_74_V9', cms.PSet(
                test = cms.PSet(
                    type = cms.string('test_analyzer'),
                    prefix = cms.string('test_'),
                    enable = cms.bool(True)
                    )
                )
            )
    """

    parseCommandLine = False
    import sys
    for argv in sys.argv:
        if 'globalTag' in argv or 'era' in argv or 'process' in argv:
            parseCommandLine = True
            break

    miniaod_process_name = process_name
    if parseCommandLine:
        from FWCore.ParameterSet.VarParsing import VarParsing
        options = VarParsing()
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

        if options.globalTag:
            globalTag = options.globalTag

        if options.era:
            assert options.era == '25ns' or options.era == '50ns'
            if options.era == '25ns':
                era = eras.Run2_25ns
            else:
                era = eras.Run2_50ns

        if options.process:
            miniaod_process_name = options.process



    if globalTag is None:
        raise Exception("No global tag specified. Use the 'globalTag' command line argument to set one.\n\tcmsRun <configuration> globalTag=<global_tag>")

    if era is None:
        raise Exception("No era specified. Use the 'era' command line argument to set one.\n\tcmsRun <configuration> era=[25ns|50ns]")

    print("")
    print("CP3 llbb framework:")
    print("\t - Running over %s" % ("data" if isData else "simulation"))
    print("\t - global tag: %s" % globalTag)
    print("\t - era: %s" % ("25ns" if era == eras.Run2_25ns else "50ns"))
    print("")

    process = cms.Process("ETM", era)

    process.options = cms.untracked.PSet(
            wantSummary = cms.untracked.bool(False),
            allowUnscheduled = cms.untracked.bool(True)
            )

    # Create an empty PSet for communication with GridIn
    process.gridin = cms.PSet(
            input_files = cms.vstring()
            )

    if JECDatabase:
        # Read the JEC from a database
        from cp3_llbb.Framework.Tools import load_jec_from_db
        algo_sizes = {'ak': [4, 8]}
        jet_types = ['pf', 'pfchs', 'puppi']
        jet_algos = []
        for k, v in algo_sizes.iteritems():
            for s in v:
                for j in jet_types:
                    jet_algos.append(str(k.upper() + str(s) + j.upper().replace("CHS", "chs").replace("PUPPI", "PFPuppi")))

        load_jec_from_db(process, JECDatabase, jet_algos)

    # Flags
    createNoHFMet = True

    process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff")
    process.load("Configuration.EventContent.EventContent_cff")
    process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
    process.load('Configuration.StandardSequences.MagneticField_38T_cff')

    process.GlobalTag.globaltag = globalTag

    process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(20))
    process.source = cms.Source("PoolSource")

    # Electron ID recipe
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import DataFormat, switchOnVIDElectronIdProducer, setupAllVIDIdsInModule, setupVIDElectronSelection
    dataFormat = DataFormat.MiniAOD
    switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)

    id_modules = [
            'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_50ns_V2_cff',
            'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff',
            'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Spring15_25ns_nonTrig_V1_cff',
            'RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV60_cff'
            ]
    for idmod in id_modules:
        setupAllVIDIdsInModule(process, idmod, setupVIDElectronSelection)

    # Services
    process.load('FWCore.MessageLogger.MessageLogger_cfi')
    process.MessageLogger.cerr.FwkReport.reportEvery = 100

    bTagDiscriminators = [
            'pfCombinedInclusiveSecondaryVertexV2BJetTags', 'pfJetProbabilityBJetTags', 'pfCombinedMVABJetTags',
            'pfSimpleSecondaryVertexHighEffBJetTags', 'pfSimpleSecondaryVertexHighPurBJetTags'
            ]

    if redoJEC:
        from cp3_llbb.Framework.Tools import setup_jets_mets_
        setup_jets_mets_(process, isData, bTagDiscriminators, createNoHFMet=createNoHFMet);

    # Producers
    from cp3_llbb.Framework import EventProducer
    from cp3_llbb.Framework import GenParticlesProducer
    from cp3_llbb.Framework import HLTProducer
    from cp3_llbb.Framework import JetsProducer
    from cp3_llbb.Framework import FatJetsProducer
    from cp3_llbb.Framework import METProducer
    from cp3_llbb.Framework import MuonsProducer
    from cp3_llbb.Framework import ElectronsProducer
    from cp3_llbb.Framework import VerticesProducer

    output = 'output_mc.root' if not isData else 'output_data.root'

    process.framework = cms.EDProducer("ExTreeMaker",
            output = cms.string(output),

            filters = cms.PSet(
                ),

            producers = cms.PSet(

                event = EventProducer.default_configuration,

                hlt = HLTProducer.default_configuration,

                jets = JetsProducer.default_configuration,

                fat_jets = FatJetsProducer.default_configuration,

                met = METProducer.default_configuration,

                muons = MuonsProducer.default_configuration,

                electrons = ElectronsProducer.default_configuration,

                vertices = VerticesProducer.default_configuration,
                ),

            analyzers = analyzers
            )

    process.framework.producers.jets.parameters.cut = cms.untracked.string("pt > 10")
    process.framework.producers.jets.parameters.btags = cms.untracked.vstring(bTagDiscriminators)

    process.framework.producers.muons.parameters.applyRochester = cms.untracked.bool(True)

    process.framework.producers.fat_jets.parameters.cut = cms.untracked.string("pt > 200")

    if era == eras.Run2_25ns:
        process.framework.producers.electrons.parameters.ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt')
    else:
        process.framework.producers.electrons.parameters.ea_R03 = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_50ns.txt')

    if createNoHFMet:
        process.framework.producers.nohf_met = cms.PSet(METProducer.default_configuration.clone())
        process.framework.producers.nohf_met.prefix = 'nohf_met_'
        process.framework.producers.nohf_met.parameters.met = cms.untracked.InputTag('slimmedMETsNoHF')

    path = cms.Path()

    if not isData:
        process.framework.producers.gen_particles = GenParticlesProducer.default_configuration
    else:
        # MET Filters

        from cp3_llbb.Framework import METFilter
        process.framework.filters.met = METFilter.default_configuration

        process.load('CommonTools.RecoAlgos.HBHENoiseFilterResultProducer_cfi')
        process.filterOnHBHENoiseFilter = cms.EDFilter('BooleanFlagFilter',
           inputLabel = cms.InputTag('HBHENoiseFilterResultProducer', 'HBHENoiseFilterResult'),
           reverseDecision = cms.bool(False)
        )

        path += cms.Sequence(process.filterOnHBHENoiseFilter) 

    if redoJEC:
        process.framework.producers.jets.parameters.jets = cms.untracked.InputTag('selectedPatJetsAK4PFCHS')

    path += cms.Sequence(
            process.egmGsfElectronIDSequence *
            process.framework
            )

    process.p = path

    if miniaod_process_name:
        print("")
        print("Changing process name from %r to %r..." % ('PAT', miniaod_process_name))
        change_process_name_(process.framework, 'PAT', miniaod_process_name)


    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    #! Output and Log
    #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    print("")
    return process

def schedule(process, analyzers=None, producers=None):
    """Inform the framework of the desired scheduling of the analyzers

    Args:
        process (cms.Process): the current framework process, return by the ``create`` method
        analyzers (string tuple): a string array representing the desired scheduling of the analyzers. Analyzers will be executed in the specified order.
        producers (string tuple): a string array representing the desired scheduling of the producers. Producers will be executed in the specified order.
    """

    if analyzers and len(analyzers) > 0:
        framework_analyzers = process.framework.analyzers.parameterNames_()
        for a in framework_analyzers:
            if not a in analyzers:
                raise Exception('Analyzer %r not found in your scheduling array. Please add it somewhere suitable for you.' % a)

        if len(analyzers) != len(framework_analyzers):
            raise Exception('Your scheduling array contains a different number of analyzers than the framework (%d vs %d)' % (len(analyzers), len(framework_analyzers)))

        process.framework.analyzers_scheduling = cms.untracked.vstring(analyzers)

    if producers and len(producers) > 0:
        framework_producers = process.framework.producers.parameterNames_()
        for a in framework_producers:
            if not a in producers:
                raise Exception('Producer %r not found in your scheduling array. Please add it somewhere suitable for you.' % a)

        if len(producers) != len(framework_producers):
            raise Exception('Your scheduling array contains a different number of producers than the framework (%d vs %d)' % (len(producers), len(framework_producers)))

        process.framework.producers_scheduling = cms.untracked.vstring(producers)

    return process
