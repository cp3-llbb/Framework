#ifndef TAUS_PRODUCER
#define TAUS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/ScaleFactors.h>

#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/PatCandidates/interface/Tau.h>
#include <DataFormats/PatCandidates/interface/TauPFSpecific.h>

#include <utility>

class TausProducer: public LeptonsProducer<pat::Tau>, public ScaleFactors {
    public:
        TausProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            LeptonsProducer(name, tree, config), ScaleFactors(const_cast<ROOT::TreeGroup&>(tree))
        {
            ScaleFactors::create_branches(config);
        }

        virtual ~TausProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            LeptonsProducer::doConsumes(config, std::forward<edm::ConsumesCollector>(collector));

            m_vertices_token = collector.consumes<std::vector<reco::Vertex>>(config.getUntrackedParameter<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:
        // Tokens
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;
    public:
        // Tree members
        std::vector<bool>& decayModeFinding = tree["decayModeFinding"].write<std::vector<bool>>();
        std::vector<bool>& decayModeFindingNewDMs = tree["decayModeFindingNewDMs"].write<std::vector<bool>>();

        //--- Tau ID
        //Delta Beta Iso WPs
        BRANCH(byLooseCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
        BRANCH(byMediumCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
        BRANCH(byTightCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
        //Delta Beta Iso WPs (cone=0.3)
        // BRANCH(byLooseCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
        // BRANCH(byMediumCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
        // BRANCH(byTightCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
        // MVA(oldDM) Iso WPs
        BRANCH(byVLooseIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
        BRANCH(byLooseIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
        BRANCH(byMediumIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
        BRANCH(byTightIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
        BRANCH(byVTightIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
        // MVA(oldDM) Iso WPs (cone=0.3)
        BRANCH(byVLooseIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
        BRANCH(byLooseIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
        BRANCH(byMediumIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
        BRANCH(byTightIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
        BRANCH(byVTightIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
        // MVA(newDM) Iso WPs
        BRANCH(byVLooseIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
        BRANCH(byLooseIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
        BRANCH(byMediumIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
        BRANCH(byTightIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
        BRANCH(byVTightIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
        // Against muons rejection WPs
        BRANCH(againstMuonLoose3, std::vector<bool>);
        BRANCH(againstMuonTight3, std::vector<bool>);
        // Against electrons rejections WPs
        BRANCH(againstElectronVLooseMVA6, std::vector<bool>);
        BRANCH(againstElectronLooseMVA6, std::vector<bool>);
        BRANCH(againstElectronMediumMVA6, std::vector<bool>);
        BRANCH(againstElectronTightMVA6, std::vector<bool>);
        BRANCH(againstElectronVTightMVA6, std::vector<bool>);


        BRANCH(decayMode, std::vector<float>);
        // BRANCH(dz, std::vector<float>);
        // BRANCH(dca, std::vector<float>);
};

#endif
