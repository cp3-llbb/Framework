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

def change_input_tag_(input_tag, from_, to_, parameter_name, padding=''):
    if input_tag.getModuleLabel() == from_:
        old_input_tag = input_tag.value()
        input_tag.setModuleLabel(to_)
        print("%sChanging value of parameter %s (input tag) from %r to %r" % (padding, parameter_name, old_input_tag, input_tag.value()))

    return input_tag

def change_string_(string_, from_, to_, parameter_name, padding=''):
    if string_.value() == from_:
        old_string = string_.value()
        string_.setValue(to_)
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

def configure_slimmedmet_(met):
    del met.t01Variation
    del met.t1Uncertainties
    del met.t1SmearedVarsAndUncs
    del met.tXYUncForRaw
    del met.tXYUncForT1
    del met.tXYUncForT01
    del met.tXYUncForT1Smear
    del met.tXYUncForT01Smear

def add_ak4_chs_jets_(process, isData, bTagDiscriminators):
    from JMEAnalysis.JetToolbox.jetToolbox_cff import jetToolbox
    jetToolbox(process, 'ak4', 'ak4CHSJetSequence', 'out', PUMethod='CHS', runOnMC=not isData, miniAOD=True, addPUJetID=False, bTagDiscriminators=bTagDiscriminators)

    if (hasattr(process, 'softPFElectronsTagInfosAK4PFCHS')):
        process.softPFElectronsTagInfosAK4PFCHS.electrons = cms.InputTag('slimmedElectrons')

    if (hasattr(process, 'softPFMuonsTagInfosAK4PFCHS')):
        process.softPFMuonsTagInfosAK4PFCHS.muons = cms.InputTag('slimmedMuons')

def setup_jets_mets_(process, isData, bTagDiscriminators, createNoHFMet=False):

    # Jets

    add_ak4_chs_jets_(process, isData, bTagDiscriminators)

    # b-tagging information. From
    # https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_X/PhysicsTools/PatAlgos/python/slimming/miniAOD_tools.py#L130
    process.patJetsAK4PFCHS.userData.userFunctions = cms.vstring(
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).p4.M):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).numberOfSourceCandidatePtrs):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").flightDistance(0).value):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").flightDistance(0).significance):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).p4.x):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).p4.y):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).p4.z):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).vertex.x):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).vertex.y):(0)',
            '?(tagInfoCandSecondaryVertex("pfSecondaryVertex").nVertices()>0)?(tagInfoCandSecondaryVertex("pfSecondaryVertex").secondaryVertex(0).vertex.z):(0)',
            )
    process.patJetsAK4PFCHS.userData.userFunctionLabels = cms.vstring('vtxMass','vtxNtracks','vtx3DVal','vtx3DSig','vtxPx','vtxPy','vtxPz','vtxPosX','vtxPosY','vtxPosZ')
    process.patJetsAK4PFCHS.tagInfoSources = cms.VInputTag(cms.InputTag("pfSecondaryVertexTagInfosAK4PFCHS"))
    process.patJetsAK4PFCHS.addTagInfos = cms.bool(True)

    # Pile-up jet id
    process.load('RecoJets.JetProducers.PileupJetID_cfi')
    process.pileupJetId.applyJec = False
    process.pileupJetId.vertexes = cms.InputTag('offlineSlimmedPrimaryVertices')
    process.patJetsAK4PFCHS.userData.userFloats.src = [ cms.InputTag("pileupJetId:fullDiscriminant"), ]

    # MET
    from PhysicsTools.PatAlgos.tools.metTools import addMETCollection

    ## Gen MET
    if not isData:
        process.genMetExtractor = cms.EDProducer("GenMETExtractor",
                metSource = cms.InputTag("slimmedMETs", "" , cms.InputTag.skipCurrentProcess())
                )

    # MET is done from all PF candidates, and Type-I corrections are computed from CHS ak4 PF jets
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections#type_1_PF_MET_recommended

    ## Raw PF METs
    process.load('RecoMET.METProducers.PFMET_cfi')

    process.pfMet.src = cms.InputTag('packedPFCandidates')
    addMETCollection(process, labelName='patPFMet', metSource='pfMet') # RAW MET
    process.patPFMet.addGenMET = False

    ## Type 1 corrections
    process.load('JetMETCorrections.Configuration.JetCorrectors_cff')
    from JetMETCorrections.Type1MET.correctionTermsPfMetType1Type2_cff import corrPfMetType1
    from JetMETCorrections.Type1MET.correctedMet_cff import pfMetT1

    process.corrPfMetType1 = corrPfMetType1.clone(
        src = 'ak4PFJetsCHS',
        jetCorrLabel = 'ak4PFCHSL1FastL2L3Corrector' if not isData else 'ak4PFCHSL1FastL2L3ResidualCorrector',
        offsetCorrLabel = 'ak4PFCHSL1FastjetCorrector',
        type1JetPtThreshold = cms.double(15.0)
    )
    process.pfMetT1 = pfMetT1.clone(
        src = 'pfMet',
        srcCorrections = [cms.InputTag("corrPfMetType1", "type1")]
    )

    addMETCollection(process, labelName='patMET', metSource='pfMetT1') # T1 MET
    process.patMET.addGenMET = False

    ## Slimmed METs

    from PhysicsTools.PatAlgos.slimming.slimmedMETs_cfi import slimmedMETs
    #### CaloMET is not available in MiniAOD
    del slimmedMETs.caloMET

    process.slimmedMETs = slimmedMETs.clone()

    process.patMET.addGenMET = not isData
    if not isData:
        process.patMET.genMETSource = cms.InputTag("genMetExtractor")
    process.slimmedMETs.src = cms.InputTag("patMET")
    process.slimmedMETs.rawVariation = cms.InputTag("patPFMet") # only central value

    # Only central values are available
    configure_slimmedmet_(process.slimmedMETs)

    if createNoHFMet:
        process.noHFCands = cms.EDFilter("CandPtrSelector",
                src=cms.InputTag("packedPFCandidates"),
                cut=cms.string("abs(pdgId)!=1 && abs(pdgId)!=2 && abs(eta)<3.0")
                )

        process.pfMetNoHF = process.pfMet.clone()

        process.pfMetNoHF.calculateSignificance = False
        process.pfMetNoHF.src = cms.InputTag('noHFCands')
        addMETCollection(process, labelName='patPFMetNoHF', metSource='pfMetNoHF') # RAW MET
        process.patPFMetNoHF.addGenMET = False

        ## Type 1 corrections
        process.pfMetT1NoHF = pfMetT1.clone(
            src = 'pfMetNoHF',
            srcCorrections = [cms.InputTag("corrPfMetType1", "type1")]
        )

        addMETCollection(process, labelName='patMETNoHF', metSource='pfMetT1NoHF') # T1 MET
        process.patMETNoHF.addGenMET = False

        process.slimmedMETsNoHF = slimmedMETs.clone()

        # Create MET from Type 1 PF collection
        process.patMETNoHF.addGenMET = not isData
        if not isData:
            process.patMETNoHF.genMETSource = cms.InputTag("genMetExtractor")
        process.slimmedMETsNoHF.src = cms.InputTag("patMETNoHF")
        process.slimmedMETsNoHF.rawVariation = cms.InputTag("patPFMetNoHF") # only central value

        # Only central values are available
        configure_slimmedmet_(process.slimmedMETsNoHF)


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
