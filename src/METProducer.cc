#include <cp3_llbb/Framework/interface/METProducer.h>

void METProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::MET>> met_handle;
    event.getByToken(m_met_token, met_handle);

    edm::Handle<ROOT::Math::SMatrix<double,2,2,ROOT::Math::MatRepSym<double,2> >> metCov_handle;
    event.getByToken(m_metCov_token,metCov_handle);

    const pat::MET& met = (*met_handle)[0];
    const auto& metCov  = (*metCov_handle);

    p4 = LorentzVector(met.pt(), met.eta(), met.phi(), met.energy());

    sumEt = met.sumEt();
    significance = met.metSignificance();

    CovMatrix00 = metCov[0][0];
    CovMatrix01 = metCov[0][1];
    CovMatrix10 = metCov[1][0];
    CovMatrix11 = metCov[1][1];

    if (m_slimmed) {
        uncorrectedPt = met.uncorPt();
        uncorrectedPhi = met.uncorPhi();
        uncorrectedSumEt = met.uncorSumEt();
    }
}
