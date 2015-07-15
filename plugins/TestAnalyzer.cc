#include <cp3_llbb/Framework/interface/TestAnalyzer.h>

//#include <cp3_llbb/Framework/interface/GenParticlesProducer.h>
#include <cp3_llbb/Framework/interface/JetsProducer.h>


void TestAnalyzer::analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager& producers) {

//    const GenParticlesProducer& gp = dynamic_cast<const GenParticlesProducer&>(producers.get("gen_particles"));
    const JetsProducer& jets = dynamic_cast<const JetsProducer&>(producers.get("jets"));
/*
    for (auto p4: gp.packed_p4) {
        std::cout << "Packed gen particle pt: " << p4.Pt() << std::endl;
    }

    for (auto p4: jets.p4) {
        std::cout << "Jet pt: " << p4.P() << std::endl;
    }
*/
}
