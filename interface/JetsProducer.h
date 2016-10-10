#ifndef JETS_PRODUCER
#define JETS_PRODUCER

#include <cp3_llbb/Framework/interface/CandidatesProducer.h>
#include <cp3_llbb/Framework/interface/BTaggingScaleFactors.h>

#include <DataFormats/PatCandidates/interface/Jet.h>
#include "TMVA/Reader.h"


class JetsProducer: public CandidatesProducer<pat::Jet>, public BTaggingScaleFactors {
    public:
        JetsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            CandidatesProducer(name, tree, config), BTaggingScaleFactors(const_cast<ROOT::TreeGroup&>(tree))
        {
            BTaggingScaleFactors::create_branches(config);

            if (config.exists("btags")) {
                const std::vector<std::string>& btags = config.getUntrackedParameter<std::vector<std::string>>("btags");

                // Create branches 
                for (const std::string& btag: btags) {
                    m_btag_discriminators.emplace(btag, &CandidatesProducer<pat::Jet>::tree[btag].write<std::vector<float>>());
                }
            }
            if (config.exists("computeRegression")) {
                computeRegression = config.getUntrackedParameter<bool>("computeRegression");
                if (computeRegression)
                {
                    regressionFile = config.getUntrackedParameter<edm::FileInPath>("regressionFile").fullPath();
                    std::cout << "  -> storing bjet energy regression, with xml file " << regressionFile << std::endl;
                    bjetRegressionReader.reset(new TMVA::Reader());
                    bjetRegressionReader->AddVariable("Jet_pt", &Jet_pt);
                    bjetRegressionReader->AddVariable("nPVs", &nPVs);
                    bjetRegressionReader->AddVariable("Jet_eta", &Jet_eta);
                    bjetRegressionReader->AddVariable("Jet_mt", &Jet_mt);
                    bjetRegressionReader->AddVariable("Jet_leadTrackPt", &Jet_leadTrackPt);
                    bjetRegressionReader->AddVariable("Jet_leptonPtRel", &Jet_leptonPtRel);
                    bjetRegressionReader->AddVariable("Jet_leptonPt", &Jet_leptonPt);
                    bjetRegressionReader->AddVariable("Jet_leptonDeltaR", &Jet_leptonDeltaR);
                    bjetRegressionReader->AddVariable("Jet_neHEF", &Jet_neHEF);
                    bjetRegressionReader->AddVariable("Jet_neEmEF", &Jet_neEmEF);
                    bjetRegressionReader->AddVariable("Jet_vtxPt", &Jet_vtxPt);
                    bjetRegressionReader->AddVariable("Jet_vtxMass", &Jet_vtxMass);
                    bjetRegressionReader->AddVariable("Jet_vtx3dL", &Jet_vtx3dL);
                    bjetRegressionReader->AddVariable("Jet_vtxNtrk", &Jet_vtxNtrk);
                    bjetRegressionReader->AddVariable("Jet_vtx3deL", &Jet_vtx3deL);
                    bjetRegressionReader->BookMVA("BDT::BDTG", regressionFile.c_str());
                }
            } else {
                computeRegression = false;
            }
        }

        virtual ~JetsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_jets_token = collector.consumes<std::vector<pat::Jet>>(config.getUntrackedParameter<edm::InputTag>("jets", edm::InputTag("slimmedJets")));
            m_vertices_token = collector.consumes<std::vector<reco::Vertex>>(config.getUntrackedParameter<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;


        float getBTagDiscriminant(size_t index, const std::string& name) const {
            return m_btag_discriminators.at(name)->at(index);
        }

        float get_scale_factor(Algorithm algo, const std::string& wp, size_t index, Variation variation = Variation::Nominal);

    private:

        // Tokens
        edm::EDGetTokenT<std::vector<pat::Jet>> m_jets_token;
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;

        std::map<std::string, std::vector<float>*> m_btag_discriminators;
        // regression stuff
        bool computeRegression;
        std::string regressionFile;
        std::unique_ptr<TMVA::Reader> bjetRegressionReader;
        float Jet_pt;
        float nPVs;
        float Jet_eta;
        float Jet_mt;
        float Jet_leadTrackPt;
        float Jet_leptonPtRel;
        float Jet_leptonPt;
        float Jet_leptonDeltaR;
        float Jet_neHEF;
        float Jet_neEmEF;
        float Jet_vtxPt;
        float Jet_vtxMass;
        float Jet_vtx3dL;
        float Jet_vtxNtrk;
        float Jet_vtx3deL;
    public:
        // Tree members
        std::vector<float>& area = tree["area"].write<std::vector<float>>();
        std::vector<int8_t>& partonFlavor = tree["partonFlavor"].write<std::vector<int8_t>>();
        std::vector<int8_t>& hadronFlavor = tree["hadronFlavor"].write<std::vector<int8_t>>();

        // Systematics flavor: 1 for heavy jets, 2 for light jets
        std::vector<int8_t>& systFlavor = tree["systFlavor"].write<std::vector<int8_t>>();

        std::vector<float>& jecFactor = tree["jecFactor"].write<std::vector<float>>();
        std::vector<float>& puJetID = tree["puJetID"].write<std::vector<float>>();
        // Variables needed for 74X b-jet energy regression as of January 26th 2016
        TRANSIENT_BRANCH(neutralHadronEnergyFraction, std::vector<float>);
        TRANSIENT_BRANCH(neutralEmEnergyFraction, std::vector<float>);
        TRANSIENT_BRANCH(vtxMass, std::vector<float>);
        TRANSIENT_BRANCH(vtx3DVal, std::vector<float>);
        TRANSIENT_BRANCH(vtx3DSig, std::vector<float>);
        TRANSIENT_BRANCH(vtxPt, std::vector<float>);
        TRANSIENT_BRANCH(vtxNtracks, std::vector<float>);
        TRANSIENT_BRANCH(leptonPtRel, std::vector<float>);
        TRANSIENT_BRANCH(leptonPt, std::vector<float>);
        TRANSIENT_BRANCH(leptonDeltaR, std::vector<float>);
        TRANSIENT_BRANCH(leadTrackPt, std::vector<float>);
        BRANCH(regPt, std::vector<float>);

        BRANCH(passLooseID, std::vector<bool>);
        BRANCH(passTightID, std::vector<bool>);
        BRANCH(passTightLeptonVetoID, std::vector<bool>);
};

#endif
