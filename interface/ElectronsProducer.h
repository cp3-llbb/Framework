#ifndef ELECTRONS_PRODUCER
#define ELECTRONS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/Identifiable.h>
#include <cp3_llbb/Framework/interface/ScaleFactors.h>

#include <DataFormats/PatCandidates/interface/Electron.h>

class ElectronsProducer: public LeptonsProducer<pat::Electron>, public Identifiable, public ScaleFactors {
    public:
        ElectronsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            LeptonsProducer(name, tree, config), Identifiable(const_cast<ROOT::TreeGroup&>(tree)), ScaleFactors(const_cast<ROOT::TreeGroup&>(tree))
        {
            ScaleFactors::create_branches(config);
        }

        virtual ~ElectronsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            LeptonsProducer::doConsumes(config, std::forward<edm::ConsumesCollector>(collector));
            Identifiable::consumes_id_tokens(config, std::forward<edm::ConsumesCollector>(collector));
            m_vertices_token = collector.consumes<std::vector<reco::Vertex>>(config.getUntrackedParameter<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices")));

            if (config.exists("mva_id")) {
                const edm::ParameterSet& mva_id_pset = config.getUntrackedParameter<edm::ParameterSet>("mva_id");
                m_mva_id_values_map_token = collector.consumes<edm::ValueMap<float>>(mva_id_pset.getUntrackedParameter<edm::InputTag>("values"));
                m_mva_id_categories_map_token = collector.consumes<edm::ValueMap<int>>(mva_id_pset.getUntrackedParameter<edm::InputTag>("categories"));
            }
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:
        // Tokens
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;

        // MVA values and categories (optional)
        edm::EDGetTokenT<edm::ValueMap<float>> m_mva_id_values_map_token;
        edm::EDGetTokenT<edm::ValueMap<int>> m_mva_id_categories_map_token;

    public:
        // Tree members
        BRANCH(isEB, std::vector<bool>); 
        BRANCH(isEE, std::vector<bool>); 

        BRANCH(dxy, std::vector<float>);
        BRANCH(dz, std::vector<float>);
        BRANCH(dca, std::vector<float>);

        BRANCH(ecalPFClusterIso, std::vector<float>);
        BRANCH(hcalPFClusterIso, std::vector<float>);
        BRANCH(trackIso, std::vector<float>);

        BRANCH(expectedMissingInnerHits, std::vector<unsigned int>);

        BRANCH(mva_id_values, std::vector<float>);
        BRANCH(mva_id_categories, std::vector<int>);

        BRANCH(phase2_mva_id_values, std::vector<float>);
        BRANCH(phase2_mva_id_loose, std::vector<bool>);
        BRANCH(phase2_mva_id_medium, std::vector<bool>);
        BRANCH(phase2_mva_id_tight, std::vector<bool>);

        std::vector<edm::Ref<std::vector<pat::Electron>>> products;
};

#endif
