#ifndef EVENT_PRODUCER
#define EVENT_PRODUCER

#include <cp3_llbb/Framework/interface/Producer.h>
#include <cp3_llbb/Framework/interface/PUReweighter.h>
#include <cp3_llbb/Framework/interface/GenInfoAndWeights.h>

#include <SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h>

#include <DataFormats/DetId/interface/DetIdCollection.h>

class EventProducer: public Framework::Producer {
    public:
        EventProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            Producer(name, tree, config)
        {

            if (config.getUntrackedParameter<bool>("compute_pu_weights", true)) {
                std::string puProfileString = config.getUntrackedParameter<std::string>("pu_profile");
                Framework::PUProfile puProfile = Framework::stringToPUProfile(puProfileString); // Throw if invalid
                m_pu_reweighter = std::make_shared<Framework::PUReweighter>(config.getParameterSet("pu_reweighter"), puProfile);
                m_pu_reweighter_up = std::make_shared<Framework::PUReweighter>(config.getParameterSet("pu_reweighter_up"), puProfile);
                m_pu_reweighter_down = std::make_shared<Framework::PUReweighter>(config.getParameterSet("pu_reweighter_down"), puProfile);
            }
        }

        virtual ~EventProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_rho_token = collector.consumes<double>(config.getUntrackedParameter<edm::InputTag>("rho", edm::InputTag("fixedGridRhoFastjetAll")));
            m_pu_info_token = collector.consumes<std::vector<PileupSummaryInfo>>(config.getUntrackedParameter<edm::InputTag>("pu_summary", edm::InputTag("addPileupInfo")));

            m_genInfoWeightsToken = collector.consumes<Framework::GenInfoAndWeights>(config.getUntrackedParameter<edm::InputTag>("genInfoWeights", edm::InputTag("genInfoEventWeights")));

            m_dupECALClusters_token = collector.consumes<bool>(edm::InputTag("particleFlowEGammaGSFixed", "dupECALClusters"));
            m_hitsNotReplaced_token = collector.consumes<DetIdCollection>(edm::InputTag("ecalMultiAndGSGlobalRecHitEB", "hitsNotReplaced"));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:

        // Tokens
        edm::EDGetTokenT<double> m_rho_token;
        edm::EDGetTokenT<std::vector<PileupSummaryInfo>> m_pu_info_token;

        edm::EDGetTokenT<Framework::GenInfoAndWeights> m_genInfoWeightsToken;

        // Moriond17 reminiaod validation flags
        edm::EDGetTokenT<bool> m_dupECALClusters_token;
        edm::EDGetTokenT<DetIdCollection> m_hitsNotReplaced_token;

        std::shared_ptr<Framework::PUReweighter> m_pu_reweighter;
        std::shared_ptr<Framework::PUReweighter> m_pu_reweighter_up;
        std::shared_ptr<Framework::PUReweighter> m_pu_reweighter_down;

    public:
        // Tree members

        BRANCH(run, ULong64_t);
        BRANCH(lumi, ULong64_t);
        BRANCH(event, ULong64_t);
        BRANCH(is_data, bool);

        BRANCH(rho, float);

        BRANCH(npu, int);
        BRANCH(true_interactions, float);
        BRANCH(pu_weight, float);
        BRANCH(pu_weight_up, float);
        BRANCH(pu_weight_down, float);

        // Moriond17 reminiaod validation flags
        BRANCH(dupECALClusters, bool);
        BRANCH(hitsNotReplacedEmpty, bool);

        BRANCH(pt_hat, float);
        BRANCH(ht, float);
        BRANCH(weight, float);

        BRANCH(alpha_QCD, float);
        BRANCH(alpha_QED, float);
        BRANCH(q_scale, float);
        uint32_t& pdf_set = tree["pdf_set"].write<uint32_t>(false);
        BRANCH(pdf_id, std::pair<int, int>);
        BRANCH(pdf_x, std::pair<float, float>);
        BRANCH(pdf_weight, float);
        BRANCH(pdf_weight_up, float);
        BRANCH(pdf_weight_down, float);
        BRANCH(hdamp_weight_up, float);
        BRANCH(hdamp_weight_down, float);

        BRANCH(initial_state, uint8_t);

        BRANCH(pdf_weight_gg, float);
        BRANCH(pdf_weight_gg_up, float);
        BRANCH(pdf_weight_gg_down, float);

        BRANCH(pdf_weight_qq, float);
        BRANCH(pdf_weight_qq_up, float);
        BRANCH(pdf_weight_qq_down, float);

        BRANCH(pdf_weight_qg, float);
        BRANCH(pdf_weight_qg_up, float);
        BRANCH(pdf_weight_qg_down, float);

        BRANCH(n_ME_partons, int);
        BRANCH(n_ME_partons_filtered, int);

        BRANCH(lhe_originalXWGTUP, float);
        BRANCH(lhe_SCALUP, float);
        TRANSIENT_BRANCH(lhe_weights, std::vector<std::pair<std::string, float>>);

        BRANCH(scale_weights, std::vector<float>);
};

#endif
