#ifndef ELECTRONS_PRODUCER
#define ELECTRONS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/Identifiable.h>

#include <DataFormats/PatCandidates/interface/Electron.h>

class ElectronsProducer: public LeptonsProducer<pat::Electron>, public Identifiable {
    public:
        ElectronsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            LeptonsProducer(name, tree, config), Identifiable(const_cast<ROOT::TreeGroup&>(tree))
        {
        }

        virtual ~ElectronsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            LeptonsProducer::doConsumes(config, std::forward<edm::ConsumesCollector>(collector));
            Identifiable::consumes_id_tokens(config, std::forward<edm::ConsumesCollector>(collector));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:

        // Tokens

    public:
        // Tree members
};

#endif
