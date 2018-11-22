#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/TausProducer.h>
#include "TLorentzVector.h"


void TausProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {
    edm::Handle<std::vector<pat::Tau>> taus;
    event.getByToken(m_leptons_token, taus);

    edm::Handle<double> rho_handle;
    event.getByToken(m_rho_token, rho_handle);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    edm::Handle<std::vector<reco::GenParticle>> genParticles_handle;
    event.getByToken(m_pruned_token, genParticles_handle);
    auto genParticles = *genParticles_handle;

    const reco::Vertex& primary_vertex = (*vertices_handle)[0];

    double rho = *rho_handle;

    for (auto tau: *taus) {
        // if (! pass_cut(tau))
        //     continue;
        fill_candidate(tau, tau.genParticle());

	// dz variable
	pat::PackedCandidate const* packedLeadTauCand = dynamic_cast<pat::PackedCandidate const*>(tau.leadChargedHadrCand().get());
	dz.push_back(packedLeadTauCand->dz());
	
	// gen truth based on Htautau convention
	// https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorking2016#MC%20Matching
	const LorentzVector tau_p4 = static_cast<LorentzVector>(tau.p4());
	auto matchResult = TausProducer::LeptonGenMatch(tau_p4, genParticles);
	gen_truth.push_back( static_cast<int>(matchResult.first) );

	// Map of all the tauID discriminators
	auto tauIDvector = tau.tauIDs();
	tauDiscriminatorMap tauIDmap;
	tauIDmap.reserve(tauIDvector.size());
	for(auto pair: tauIDvector){
		auto key = pair.first;
		auto value = pair.second;
		tauIDmap[key] = value;
	}
	IDmap.push_back(tauIDmap);


	// tau decay mode
        decayMode.push_back(tau.decayMode());

        // // Same values used for cut-based tau ID. See:
        // //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/DataFormats/MuonReco/src/MuonSelectors.cc#L756
        // dxy.push_back(tau.muonBestTrack()->dxy(primary_vertex.position()));
        // dz.push_back(tau.muonBestTrack()->dz(primary_vertex.position()));
        // dca.push_back(tau.dB(pat::tau::PV3D)/tau.edB(pat::tau::PV3D));
        //
        Parameters p {{BinningVariable::Eta, tau.eta()}, {BinningVariable::Pt, tau.pt()}};
        ScaleFactors::store_scale_factors(p, event.isRealData());
    }
}

	
MatchResult TausProducer::LeptonGenMatch(const LorentzVector& p4, const std::vector<reco::GenParticle>& genParticles)
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

