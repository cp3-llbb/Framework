#include <cp3_llbb/Framework/interface/METProducer.h>

void METProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::MET>> met_handle;
    event.getByToken(m_met_token, met_handle);

    const pat::MET& met = (*met_handle)[0];

    p4 = LorentzVector(met.pt(), met.eta(), met.phi(), met.energy());

    sumEt = met.sumEt();

    if (m_slimmed) {
        uncorrectedPt = met.uncorPt();
        uncorrectedPhi = met.uncorPhi();
        uncorrectedSumEt = met.uncorSumEt();
    }
}
