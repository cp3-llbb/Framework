#include <cp3_llbb/Framework/interface/CorrectedMuonProducerT.h>

#include <DataFormats/MuonReco/interface/Muon.h>
#include <DataFormats/PatCandidates/interface/Muon.h>

typedef CorrectedMuonProducerT<pat::Muon, cp3::RochesterCorrector> RochesterCorrectedPATMuonProducer;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(RochesterCorrectedPATMuonProducer);
