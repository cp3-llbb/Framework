#ifndef FAT_JETS_PRODUCER
#define FAT_JETS_PRODUCER

#include <cp3_llbb/Framework/interface/CandidatesProducer.h>
#include <cp3_llbb/Framework/interface/BTaggingScaleFactors.h>

#include <DataFormats/PatCandidates/interface/Jet.h>

class FatJetsProducer: public CandidatesProducer<pat::Jet>, public BTaggingScaleFactors {
    public:
        FatJetsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
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

            if (config.exists("subjets_btags")) {
                m_subjets_btag_discriminators = config.getUntrackedParameter<std::vector<std::string>>("subjets_btags");

                // Create branches
                for (const std::string& btag: m_subjets_btag_discriminators) {
                    m_softdrop_btag_discriminators_branches.emplace(btag,
                            &CandidatesProducer<pat::Jet>::tree["softdrop_" + btag].write<std::vector<std::vector<float>>>());
                    m_softdrop_puppi_btag_discriminators_branches.emplace(btag,
                            &CandidatesProducer<pat::Jet>::tree["softdrop_puppi_" + btag].write<std::vector<std::vector<float>>>());
                }
            }
            Njettinesstau1 = config.getUntrackedParameter<std::string>("Njettinesstau1", "NjettinessAK8:tau1");
            Njettinesstau2 = config.getUntrackedParameter<std::string>("Njettinesstau2", "NjettinessAK8:tau2");
            Njettinesstau3 = config.getUntrackedParameter<std::string>("Njettinesstau3", "NjettinessAK8:tau3");
            SoftDropSubjets = config.getUntrackedParameter<std::string>("SoftDropSubjets", "SoftDrop");
            SoftDropPuppiSubjets = config.getUntrackedParameter<std::string>("SoftDropPuppiSubjets", "SoftDropPuppi");
        }

        virtual ~FatJetsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_jets_token = collector.consumes<std::vector<pat::Jet>>(config.getUntrackedParameter<edm::InputTag>("jets", edm::InputTag("slimmedJetsAK8")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

        float getBTagDiscriminant(size_t index, const std::string& name) const {
            return m_btag_discriminators.at(name)->at(index);
        }

        float getSoftDropBTagDiscriminant(size_t index, size_t sj_index, const std::string& btag_name) const {
            return m_softdrop_btag_discriminators_branches.at(btag_name)->at(index).at(sj_index);
        }

        float getSoftDropPuppiBTagDiscriminant(size_t index, size_t sj_index, const std::string& btag_name) const {
            return m_softdrop_puppi_btag_discriminators_branches.at(btag_name)->at(index).at(sj_index);
        }

        float get_scale_factor(Algorithm algo, const std::string& wp, size_t index, Variation variation = Variation::Nominal);

    private:

        // Tokens
        edm::EDGetTokenT<std::vector<pat::Jet>> m_jets_token;

        std::map<std::string, std::vector<float>*> m_btag_discriminators;

        std::vector<std::string> m_subjets_btag_discriminators;
        std::map<std::string, std::vector<std::vector<float>>*> m_softdrop_btag_discriminators_branches;
        std::map<std::string, std::vector<std::vector<float>>*> m_softdrop_puppi_btag_discriminators_branches;

        std::string Njettinesstau1;
        std::string Njettinesstau2;
        std::string Njettinesstau3;
        std::string SoftDropSubjets;
        std::string SoftDropPuppiSubjets;

    public:
        // Tree members
        std::vector<float>& area = tree["area"].write<std::vector<float>>();
        std::vector<int8_t>& partonFlavor = tree["partonFlavor"].write<std::vector<int8_t>>();
        std::vector<int8_t>& hadronFlavor = tree["hadronFlavor"].write<std::vector<int8_t>>();
        
        // Systematics flavor: 1 for heavy jets, 2 for light jets
        std::vector<int8_t>& systFlavor = tree["systFlavor"].write<std::vector<int8_t>>();

        std::vector<float>& jecFactor = tree["jecFactor"].write<std::vector<float>>();

        BRANCH(passLooseID, std::vector<bool>);
        BRANCH(passTightID, std::vector<bool>);
        BRANCH(passTightLeptonVetoID, std::vector<bool>);

        BRANCH(tau1, std::vector<float>);
        BRANCH(tau2, std::vector<float>);
        BRANCH(tau3, std::vector<float>);

        BRANCH(puppi_p4, std::vector<LorentzVector>);
        BRANCH(puppi_tau1, std::vector<float>);
        BRANCH(puppi_tau2, std::vector<float>);
        BRANCH(puppi_tau3, std::vector<float>);

        BRANCH(softdrop_mass, std::vector<float>);
        BRANCH(pruned_mass, std::vector<float>);

        BRANCH(softdrop_subjets_p4, std::vector<std::vector<LorentzVector>>);
        BRANCH(softdrop_puppi_subjets_p4, std::vector<std::vector<LorentzVector>>);
};

#endif
