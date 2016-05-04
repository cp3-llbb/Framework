import os
import sys

import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

verbosity = True

class StdStreamSilenter(object):
    """
    Temporarily redirect stdout to /dev/null
    """

    def __init__(self):
        self.__stdout = sys.stdout
        self.__stderr = sys.stderr

    def __enter__(self):
        self.__stdout.flush()
        self.__stderr.flush()
        self.__devnull = open(os.devnull, 'w')

        sys.stdout = self.__devnull
        sys.stderr = self.__devnull

    def __exit__(self, exc_type, exc_value, traceback):
        self.__stdout.flush()
        self.__stderr.flush()
        self.__devnull.close()

        sys.stdout = self.__stdout
        sys.stderr = self.__stderr

def change_input_tag_process_(input_tag, process_name_from, process_name_to):
    if not isinstance(input_tag, cms.InputTag):
        input_tag = cms.untracked.InputTag(input_tag)

    if len(input_tag.getProcessName()) > 0 and input_tag.getProcessName() == process_name_from:
        old_input_tag = input_tag.value()
        input_tag.setProcessName(process_name_to)
        if verbosity:
            print("Changing input tag from %r to %r" % (old_input_tag, input_tag.value()))

    return input_tag

def change_input_tag_(input_tag, from_, to_, parameter_name, padding=''):
    if isinstance(input_tag, str):
        if input_tag == from_:
            old_input_tag = input_tag
            input_tag = to_
            if verbosity:
                print("%sChanging value of parameter %s (input tag) from %r to %r" % (padding, parameter_name, old_input_tag, input_tag))
    else:
        if input_tag.getModuleLabel() == from_:
            old_input_tag = input_tag.value()
            input_tag.setModuleLabel(to_)
            if verbosity:
                print("%sChanging value of parameter %s (input tag) from %r to %r" % (padding, parameter_name, old_input_tag, input_tag.value()))

    return input_tag

def change_string_(string_, from_, to_, parameter_name, padding=''):
    if string_.value() == from_:
        old_string = string_.value()
        string_.setValue(to_)
        if verbosity:
            print("%sChanging value of parameter %s (string) from %r to %r" % (padding, parameter_name, old_string, to_))

    return string_

def change_process_name(module, process_name_from, process_name_to):
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
                change_process_name(value, process_name_from, process_name_to)

def change_input_tags_and_strings(module, from_, to_, parameter_name, padding=''):
    if from_ == to_:
        return

    if isinstance(module, cms._Parameterizable):
        for name in module.parameters_().keys():
            value = getattr(module, name)
            type = value.pythonTypeName()
            local_parameter_name = parameter_name + '.' + name

            if 'VInputTag' in type:
                for (i, tag) in enumerate(value):
                    value[i] = change_input_tag_(tag, from_, to_, local_parameter_name, padding)
            elif 'InputTag' in type:
                change_input_tag_(value, from_, to_, local_parameter_name, padding)
            elif 'string' in type:
                value = change_string_(value, from_, to_, local_parameter_name, padding)

            if isinstance(value, cms._Parameterizable):
                change_input_tags_and_strings(value, from_, to_, local_parameter_name, padding)

def module_has_string(module, string):
    if isinstance(module, cms._Parameterizable):
        for name in module.parameters_().keys():
            value = getattr(module, name)
            type = value.pythonTypeName()

            if 'VInputTag' in type:
                for (i, tag) in enumerate(value):
                    if string in tag.value():
                        return True
            elif 'InputTag' in type:
                if string in value.value():
                    return True
            elif 'string' in type:
                if string == value.value():
                    return True

            if isinstance(value, cms._Parameterizable) and module_has_string(value, string):
                return True


    return False

def setup_jets_mets_(process, isData):
    """
    Create a new jets collection and a new MET collection with new JECs applied

    Return a tuple of newly created collections (jet, met)
    """

    # Jets
    from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection

    levels = ['L1FastJet', 'L2Relative', 'L3Absolute']
    if isData:
        levels.append('L2L3Residual')

    # Create the updated jet collection with new JEC
    # Name of the new collection: updatedPatJetsNewJEC
    updateJetCollection(
            process,
            jetSource = cms.InputTag('slimmedJets'),
            labelName = 'NewJEC',
            jetCorrections = ('AK4PFchs', cms.vstring(levels), 'None')
            )

    # Propagate JEC to MET (Type-1)
    if not isData:
        process.genMet = cms.EDProducer('GenMETExtractor',
                metSource = cms.InputTag('slimmedMETs', '', '@skipCurrentProcess')
                )

    # Raw MET
    process.uncorrectedMet = cms.EDProducer('RecoMETExtractor',
            correctionLevel = cms.string('raw'),
            metSource = cms.InputTag('slimmedMETs', '', '@skipCurrentProcess')
            )

    # Raw PAT MET
    from PhysicsTools.PatAlgos.tools.metTools import addMETCollection
    addMETCollection(process, labelName='uncorrectedPatMet', metSource='uncorrectedMet')
    if isData:
        process.uncorrectedPatMet.addGenMET = False
    else:
        process.uncorrectedPatMet.genMETSource = cms.InputTag('genMet')

    # Type-1 correction
    process.Type1CorrForNewJEC = cms.EDProducer('PATPFJetMETcorrInputProducer',
            jetCorrLabel = cms.InputTag('L3Absolute'),
            jetCorrLabelRes = cms.InputTag('L2L3Residual'),
            offsetCorrLabel = cms.InputTag('L1FastJet'),
            skipEM = cms.bool(True),
            skipEMfractionThreshold = cms.double(0.9),
            skipMuonSelection = cms.string('isGlobalMuon | isStandAloneMuon'),
            skipMuons = cms.bool(True),
            src = cms.InputTag('updatedPatJetsNewJEC'),
            type1JetPtThreshold = cms.double(15.0),
            )

    process.slimmedMETsNewJEC = cms.EDProducer('CorrectedPATMETProducer',
            src = cms.InputTag('uncorrectedPatMet'),
            srcCorrections = cms.VInputTag(cms.InputTag('Type1CorrForNewJEC', 'type1'))
            )

    return ('updatedPatJetsNewJEC', 'slimmedMETsNewJEC')


def check_tag_(db_file, tag):
    import sqlite3

    db_file = db_file.replace('sqlite:', '')
    connection = sqlite3.connect(db_file)
    res = connection.execute('select TAG_NAME from IOV where TAG_NAME=?', tag).fetchall()

    return len(res) != 0

def append_jec_to_db_(process, label, prefix):

    for set in process.jec.toGet:
        if set.label == label:
            return

    tag = 'JetCorrectorParametersCollection_%s_%s' % (prefix, label)
    if not check_tag_(process.jec.connect.value(), (tag,)):
        print("WARNING: The JEC payload %r is not present in the database you want to use. Corrections for this payload will be loaded from the Global Tag" % label)
        return

    process.jec.toGet += [cms.PSet(
            record = cms.string('JetCorrectionsRecord'),
            tag    = cms.string(tag),
            label  = cms.untracked.string(label)
            )]

def load_jec_from_db(process, db, algorithmes):
    """
    Inform CMSSW to read the JEC from a database instead of the GT for the given list of algorithmes
    """

    import os
    if not os.path.isfile(db):
        raise ValueError('Database %r does not exist.' % db)

    if os.path.isabs(db):
        raise ValueError('You cannot use an absolute for the database, as it breaks crab submission. Please put the database in the same folder as your python configuration file and pass only the filename as argument of the create function')

    process.load("CondCore.DBCommon.CondDBCommon_cfi")

    if verbosity:
        print("Using database %r for JECs\n" % db)

    process.jec = cms.ESSource("PoolDBESSource",
            DBParameters = cms.PSet(
                messageLevel = cms.untracked.int32(0)
                ),
            timetype = cms.string('runnumber'),
            toGet = cms.VPSet(),

            connect = cms.string('sqlite:%s' % db)
            )

    process.gridin.input_files += [os.path.abspath(db)]

    process.es_prefer_jec = cms.ESPrefer('PoolDBESSource', 'jec')

    prefix = os.path.splitext(db)[0]
    for algo in algorithmes:
        append_jec_to_db_(process, algo, prefix)
