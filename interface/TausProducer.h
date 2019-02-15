#ifndef TAUS_PRODUCER
#define TAUS_PRODUCER

#include <cp3_llbb/Framework/interface/CandidatesProducer.h>

#include <DataFormats/PatCandidates/interface/Tau.h>
#include <DataFormats/PatCandidates/interface/TauPFSpecific.h>

#include <utility>
#include <Math/VectorUtil.h>


enum class GenMatch { Electron = 1, Muon = 2, TauElectron = 3,  TauMuon = 4, Tau = 5, NoMatch = 6 };

class TausProducer: public CandidatesProducer<pat::Tau> {
    private:
        using MatchResult = std::pair<GenMatch, const reco::GenParticle*>;

    public:
        TausProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            CandidatesProducer(name, tree, config) 
        {
            
        }

        virtual ~TausProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            m_taus_token = collector.consumes<std::vector<pat::Tau>>(config.getUntrackedParameter<edm::InputTag>("src"));
	    m_pruned_token = collector.consumes<std::vector<reco::GenParticle>>(config.getUntrackedParameter<edm::InputTag>("pruned_gen_particles", edm::InputTag("prunedGenParticles")));
	}

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

	
    private:
        // Tokens
        edm::EDGetTokenT<std::vector<pat::Tau>> m_taus_token;
	edm::EDGetTokenT<std::vector<reco::GenParticle>> m_pruned_token;
    private:
	
        MatchResult LeptonGenMatch(const LorentzVector& p4, const std::vector<reco::GenParticle>& genParticles)
	{
		static constexpr int electronPdgId = 11, muonPdgId = 13, tauPdgId = 15;
		static constexpr double dR2_threshold = std::pow(0.2, 2);

		static const std::map<int, double> pt_thresholds = {
			{ electronPdgId, 8 }, { muonPdgId, 8 }, { tauPdgId, 15 }
		};

		using pair = std::pair<int, bool>;
		static const std::map<pair, GenMatch> genMatches = {
			{ { electronPdgId, false }, GenMatch::Electron }, { { electronPdgId, true }, GenMatch::TauElectron },
			{ { muonPdgId, false }, GenMatch::Muon }, { { muonPdgId, true }, GenMatch::TauMuon },
			{ { tauPdgId, false }, GenMatch::Tau }, { { tauPdgId, true }, GenMatch::Tau }
		};

		MatchResult result(GenMatch::NoMatch, nullptr);
		double match_dr2 = dR2_threshold;



		for(const reco::GenParticle& particle : genParticles) {
			const bool isTauProduct = particle.statusFlags().isDirectPromptTauDecayProduct();
			if((!particle.statusFlags().isPrompt() && !isTauProduct) || !particle.statusFlags().isLastCopy()) continue;

			const int abs_pdg = std::abs(particle.pdgId());
			if(!pt_thresholds.count(abs_pdg)) continue;

			const auto gen_particle_p4 = LorentzVector(particle.pt(), particle.eta(), particle.phi(), particle.energy());
			const auto particle_p4 = abs_pdg == tauPdgId ? GetFinalStateMomentum(particle, true, true) : gen_particle_p4;

			const double dr2 = ROOT::Math::VectorUtil::DeltaR2(p4, particle_p4);
			if(dr2 >= match_dr2) continue;
			if(particle_p4.pt() <= pt_thresholds.at(abs_pdg)) continue;

			match_dr2 = dr2;
			result.first = genMatches.at(pair(abs_pdg, isTauProduct));
			result.second = &particle;
		}
		return result;
	}

	inline void FindFinalStateDaughters(const reco::GenParticle& particle, std::vector<const reco::GenParticle*>& daughters,
					    const std::set<int>& pdg_to_exclude = {})
	{
	  if(particle.daughterRefVector().empty()) {
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
	  static const set light_and_invisible = Framework::union_sets({light_leptons, invisible_particles});

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
        BRANCH(decayMode, std::vector<float>);
        BRANCH(dz, std::vector<float>);
        BRANCH(gen_truth, std::vector<int>);
	BRANCH(IDmap, std::vector<tauDiscriminatorMap>);
};

#endif
