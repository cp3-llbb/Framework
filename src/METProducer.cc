#include <cp3_llbb/Framework/interface/METProducer.h>

void METProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::MET>> met_handle;
    event.getByToken(m_met_token, met_handle);


    const pat::MET& met = (*met_handle)[0];

    p4 = LorentzVector(met.pt(), met.eta(), met.phi(), met.energy());

    sumEt = met.sumEt();
    significance = met.metSignificance();

    CovMatrix00 = met.getSignificanceMatrix()[0][0];  
    CovMatrix01 = met.getSignificanceMatrix()[0][1];
    CovMatrix10 = met.getSignificanceMatrix()[1][0];
    CovMatrix11 = met.getSignificanceMatrix()[1][1];

   // std::cout << "...................................................................................." << std::endl;
   // std::cout << " MET cov reval: " << CovMatrix00 << " " << CovMatrix01 << " " << CovMatrix10 << " " << CovMatrix11 << std::endl;
   // std::cout << " MET cov nom  : " << CovMatrix00_nom << " " << CovMatrix01_nom << " " << CovMatrix10_nom << " " << CovMatrix11_nom << std::endl;
  //  std::cout << "...................................................................................." << std::endl;

  if (m_slimmed) {
        uncorrectedPt = met.uncorPt();
        uncorrectedPhi = met.uncorPhi();
        uncorrectedSumEt = met.uncorSumEt();
    }
}
