#ifndef MUONS_PRODUCER
#define MUONS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/ScaleFactors.h>

#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/PatCandidates/interface/Muon.h>

#include <cp3_llbb/Framework/interface/rochcor2015.h>

#include <utility>

class MuonsProducer: public LeptonsProducer<pat::Muon>, public ScaleFactors {
    public:
        MuonsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            LeptonsProducer(name, tree, config), ScaleFactors(const_cast<ROOT::TreeGroup&>(tree))
        {
            ScaleFactors::create_branches(config);
            if (config.exists("applyRochester")) {
                applyRochester = config.getUntrackedParameter<bool>("applyRochester");
            }
        }

        virtual ~MuonsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            LeptonsProducer::doConsumes(config, std::forward<edm::ConsumesCollector>(collector));

            m_vertices_token = collector.consumes<std::vector<reco::Vertex>>(config.getUntrackedParameter<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:
        // Tokens
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;
        bool applyRochester = false ;
    public:
        // Tree members
        std::vector<bool>& isLoose = tree["isLoose"].write<std::vector<bool>>();
        std::vector<bool>& isSoft = tree["isSoft"].write<std::vector<bool>>();
        std::vector<bool>& isMedium = tree["isMedium"].write<std::vector<bool>>();
        std::vector<bool>& isTight = tree["isTight"].write<std::vector<bool>>();
        std::vector<bool>& isHighPt = tree["isHighPt"].write<std::vector<bool>>();

        BRANCH(dxy, std::vector<float>);
        BRANCH(dz, std::vector<float>);
        rochcor2015 rmcor;
};

#endif
