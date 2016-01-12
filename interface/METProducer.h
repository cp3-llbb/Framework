#ifndef MET_PRODUCER
#define MET_PRODUCER

#include <DataFormats/PatCandidates/interface/MET.h>

#include <cp3_llbb/Framework/interface/Producer.h>

class METProducer: public Framework::Producer {
    public:
        METProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            Producer(name, tree, config),
            m_slimmed(config.getUntrackedParameter<bool>("slimmed", true))
        {

        }

        virtual ~METProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_met_token = collector.consumes<std::vector<pat::MET>>(config.getUntrackedParameter<edm::InputTag>("met", edm::InputTag("slimmedMETs")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:
        float& create_branch(const std::string& name) {
            static float s_dummy = 0;
            return (m_slimmed) ? tree[name].write<float>() : s_dummy;
        }

        bool m_slimmed;

        // Tokens
        edm::EDGetTokenT<std::vector<pat::MET>> m_met_token;

    public:
        // Tree members
        LorentzVector& p4 = tree["p4"].write<LorentzVector>();
        float& sumEt = tree["sumEt"].write<float>();

        float& uncorrectedPt = create_branch("uncorrectedPt");
        float& uncorrectedPhi = create_branch("uncorrectedPhi");
        float& uncorrectedSumEt = create_branch("uncorrectedSumEt");
};

#endif
