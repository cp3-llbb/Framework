#ifndef HLT_PRODUCER
#define HLT_PRODUCER

#include <cp3_llbb/Framework/interface/CandidatesProducer.h>
#include <cp3_llbb/Framework/interface/HLTService.h>

#include <FWCore/Common/interface/TriggerNames.h>
#include <DataFormats/Common/interface/TriggerResults.h>
#include <DataFormats/PatCandidates/interface/PackedTriggerPrescales.h>
#include <DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h>

class HLTProducer: public Framework::Producer {
    public:
        HLTProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            Producer(name, tree, config)
        {
            if (config.exists("triggers")) {
                m_hlt_service.reset(new HLTService(config.getUntrackedParameter<edm::FileInPath>("triggers").fullPath()));
                std::cout << "HLT configuration: " << std::endl;
                m_hlt_service->print();
                std::cout << std::endl;
            }
        }

        virtual ~HLTProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_hlt_token = collector.consumes<edm::TriggerResults>(config.getUntrackedParameter<edm::InputTag>("hlt", edm::InputTag("TriggerResults", "", "HLT")));
            m_prescales_token = collector.consumes<pat::PackedTriggerPrescales>(config.getUntrackedParameter<edm::InputTag>("prescales", edm::InputTag("patTrigger")));
            m_trigger_objects_token = collector.consumes<pat::TriggerObjectStandAloneCollection>(config.getUntrackedParameter<edm::InputTag>("objects", edm::InputTag("selectedPatTrigger")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:

        // Tokens
        edm::EDGetTokenT<edm::TriggerResults> m_hlt_token;
        edm::EDGetTokenT<pat::PackedTriggerPrescales> m_prescales_token;
        edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> m_trigger_objects_token;

        // Service
        std::shared_ptr<HLTService> m_hlt_service;

    public:
        // Tree members
        std::vector<std::string>& paths = tree["paths"].write<std::vector<std::string>>();
        std::vector<uint16_t>& prescales = tree["prescales"].write<std::vector<uint16_t>>();

        BRANCH(object_paths, std::vector<std::vector<std::string>>);
        TRANSIENT_BRANCH(object_filters, std::vector<std::vector<std::string>>);
        BRANCH(object_p4, std::vector<LorentzVector>);
        BRANCH(object_pdg_id, std::vector<int>);
};

#endif
