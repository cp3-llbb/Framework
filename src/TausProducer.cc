#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/TausProducer.h>
#include "TLorentzVector.h"


void TausProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {
  edm::Handle<std::vector<pat::Tau>> taus;
  event.getByToken(m_taus_token, taus);

  std::vector<reco::GenParticle> genParticles; 
  if(!event.isRealData()){
     edm::Handle<std::vector<reco::GenParticle>> genParticles_handle;
     event.getByToken(m_pruned_token, genParticles_handle);
     genParticles = *genParticles_handle;
  }

  for (auto tau: *taus) {
    fill_candidate(tau, tau.genParticle());

    // dz variable
    pat::PackedCandidate const* packedLeadTauCand = dynamic_cast<pat::PackedCandidate const*>(tau.leadChargedHadrCand().get());
    dz.push_back(packedLeadTauCand->dz());

    if(!event.isRealData()){
    // gen truth based on Htautau convention
    // https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorking2016#MC%20Matching
      const LorentzVector tau_p4 = static_cast<LorentzVector>(tau.p4());
      auto matchResult = TausProducer::LeptonGenMatch(tau_p4, genParticles);
      gen_truth.push_back( static_cast<int>(matchResult.first) );
    }
    else{
      gen_truth.push_back(-1);
    }

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

  }
}
