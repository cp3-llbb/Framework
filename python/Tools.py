import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

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
        offsetCorrLabel = 'ak4PFCHSL1FastjetCorrector'
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
