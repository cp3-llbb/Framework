#include <cp3_llbb/Framework/interface/TestAnalyzer.h>

#include <cp3_llbb/Framework/interface/GenParticlesProducer.h>
#include <cp3_llbb/Framework/interface/JetsProducer.h>

#include <cp3_llbb/Framework/interface/DileptonAnalyzer.h>


void TestAnalyzer::analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager& producers, const AnalyzersManager& analyzers, const CategoryManager& categories) {

    const JetsProducer& jets = producers.get<JetsProducer>("jets");

    const DileptonAnalyzer& di = analyzers.get<DileptonAnalyzer>("dilepton");

/*
    if (producers.exists("gen_particles")) {
        const GenParticlesProducer& gp = dynamic_cast<const GenParticlesProducer&>(producers.get("gen_particles"));
        for (auto p4: gp.packed_p4) {
            std::cout << "Packed gen particle pt: " << p4.Pt() << std::endl;
        }
    }
*/

/*
    for (auto p4: jets.p4) {
        std::cout << "Jet pt: " << p4.P() << std::endl;
    }
*/

    // Test if event is in category
    bool in_category = categories.in_category("two_muons");
    if (in_category) {

    }

    // Get category metadata
    std::shared_ptr<CategoryMetadata> metadata = categories.get("two_muons").get_metadata();
}
