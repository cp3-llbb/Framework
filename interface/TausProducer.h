#ifndef TAUS_PRODUCER
#define TAUS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/ScaleFactors.h>

#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/PatCandidates/interface/Tau.h>
#include <DataFormats/PatCandidates/interface/TauPFSpecific.h>

#include <utility>
#include <Math/VectorUtil.h>

typedef std::unordered_map<std::string, float> tauDiscriminatorMap;
typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiE4D<float> > LorentzVector;

enum class GenMatch { Electron = 1, Muon = 2, TauElectron = 3,  TauMuon = 4, Tau = 5, NoMatch = 6 };

namespace tools {
  template<typename Type>
    std::set<Type> union_sets(std::initializer_list<std::set<Type>> sets)
    {
      std::set<Type> result;
      for(const auto& set : sets)
	result.insert(set.begin(), set.end());
      return result;
    }
}

using MatchResult = std::pair<GenMatch, const reco::GenParticle*>;


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
	    m_pruned_token = collector.consumes<std::vector<reco::GenParticle>>(config.getUntrackedParameter<edm::InputTag>("pruned_gen_particles", edm::InputTag("prunedGenParticles")));
	}

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

	
    private:
        // Tokens
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;
	edm::EDGetTokenT<std::vector<reco::GenParticle>> m_pruned_token;
    private:
 
	MatchResult LeptonGenMatch(const LorentzVector& p4, const std::vector<reco::GenParticle>& genParticles);

	inline void FindFinalStateDaughters(const reco::GenParticle& particle, std::vector<const reco::GenParticle*>& daughters,
					    const std::set<int>& pdg_to_exclude = {})
	{
	  if(!particle.daughterRefVector().size()) {
	    const int abs_pdg = std::abs(particle.pdgId());
	    if(!pdg_to_exclude.count(abs_pdg))
	      daughters.push_back(&particle);
	  } else {
	    for(const auto& daughter : particle.daughterRefVector())
	      FindFinalStateDaughters(*daughter, daughters, pdg_to_exclude);
	  }
	}

	inline LorentzVector GetFinalStateMomentum(const reco::GenParticle& particle, bool excludeInvisible,
						   bool excludeLightLeptons)
	{
	  using set = std::set<int>;
	  using pair = std::pair<bool, bool>;
	  static const set empty = {};
	  static const set light_leptons = { 11, 13 };
	  static const set invisible_particles = { 12, 14, 16 };
	  static const set light_and_invisible = tools::union_sets({light_leptons, invisible_particles});

	  static const std::map<pair, const set*> to_exclude {
	    { pair(false, false), &empty }, { pair(true, false), &invisible_particles },
					      { pair(false, true), &light_leptons }, { pair(true, true), &light_and_invisible },
										       };

	  std::vector<const reco::GenParticle*> daughters;
	  FindFinalStateDaughters(particle, daughters, *to_exclude.at(pair(excludeInvisible, false)));

	  LorentzVector p4;
	  for(auto daughter : daughters){
	    if(excludeLightLeptons && light_leptons.count(std::abs(daughter->pdgId())) && daughter->statusFlags().isDirectTauDecayProduct()) continue;
	    p4 += daughter->p4();
	  }
	  return p4;
	}



    public:
        // Tree members
//        std::vector<bool>& decayModeFinding = tree["decayModeFinding"].write<std::vector<bool>>();
//        std::vector<bool>& decayModeFindingNewDMs = tree["decayModeFindingNewDMs"].write<std::vector<bool>>();
//
//        //--- Tau ID
//        //Delta Beta Iso WPs
//        BRANCH(byLooseCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
//        BRANCH(byMediumCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
//        BRANCH(byTightCombinedIsolationDeltaBetaCorr3Hits, std::vector<bool>);
//        //Delta Beta Iso WPs (cone=0.3)
//        // BRANCH(byLooseCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
//        // BRANCH(byMediumCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
//        // BRANCH(byTightCombinedIsolationDeltaBetaCorr3HitsdR03, std::vector<bool>);
//        // MVA(oldDM) Iso WPs
//        BRANCH(byVLooseIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
//        BRANCH(byLooseIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
//        BRANCH(byMediumIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
//        BRANCH(byTightIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
//        BRANCH(byVTightIsolationMVArun2v1DBoldDMwLT, std::vector<bool>);
//        // MVA(oldDM) Iso WPs (cone=0.3)
//        BRANCH(byVLooseIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
//        BRANCH(byLooseIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
//        BRANCH(byMediumIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
//        BRANCH(byTightIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
//        BRANCH(byVTightIsolationMVArun2v1DBdR03oldDMwLT, std::vector<bool>);
//        // MVA(newDM) Iso WPs
//        BRANCH(byVLooseIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
//        BRANCH(byLooseIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
//        BRANCH(byMediumIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
//        BRANCH(byTightIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
//        BRANCH(byVTightIsolationMVArun2v1DBnewDMwLT, std::vector<bool>);
//        // Against muons rejection WPs
//        BRANCH(againstMuonLoose3, std::vector<bool>);
//        BRANCH(againstMuonTight3, std::vector<bool>);
//        // Against electrons rejections WPs
//        BRANCH(againstElectronVLooseMVA6, std::vector<bool>);
//        BRANCH(againstElectronLooseMVA6, std::vector<bool>);
//        BRANCH(againstElectronMediumMVA6, std::vector<bool>);
//        BRANCH(againstElectronTightMVA6, std::vector<bool>);
//        BRANCH(againstElectronVTightMVA6, std::vector<bool>);
//

        BRANCH(decayMode, std::vector<float>);
        BRANCH(dz, std::vector<float>);
        BRANCH(gen_truth, std::vector<int>);
	BRANCH(IDmap, std::vector<tauDiscriminatorMap>);
};

#endif
