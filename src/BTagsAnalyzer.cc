#include <cp3_llbb/Framework/interface/BTagsAnalyzer.h>

#include <cp3_llbb/Framework/interface/JetsProducer.h>

void BTagsAnalyzer::analyze(const edm::Event& event, const edm::EventSetup&, const ProducersManager& producers, const AnalyzersManager& analyzers, const CategoryManager& categories) {
    
    const JetsProducer& jets = producers.get<JetsProducer>("jets");

    for(unsigned int ijet = 0; ijet < jets.p4.size(); ijet++) {
        if(jets.p4[ijet].Pt() > m_pt_cut && abs(jets.p4[ijet].Eta()) < m_eta_cut && jets.getBTagDiscriminant(ijet, m_discr_name) > m_discr_cut) {
            indices.push_back(ijet);
        }
    }

}

