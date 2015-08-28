
#include <cp3_llbb/Framework/interface/JetsProducer.h>

void JetsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::Jet>> jets;
    event.getByToken(m_jets_token, jets);

    for (const auto& jet: *jets) {
        if (! pass_cut(jet))
            continue;

        fill_candidate(jet, jet.genJet());

        jecFactor.push_back(jet.jecFactor(0));
        push_back("area", jet.jetArea());
        partonFlavor.push_back(jet.partonFlavour());
        hadronFlavor.push_back(jet.hadronFlavour());

        if (jet.hasUserFloat("pileupJetId:fullDiscriminant"))
            puJetID.push_back(jet.userFloat("pileupJetId:fullDiscriminant"));

        vtxMass.push_back(jet.userFloat("vtxMass"));

        auto& btag_discris = jet.getPairDiscri();
        for (auto& btag_discri: btag_discris) {
            // Get branch from tree
            std::vector<float>& branch = tree[btag_discri.first].write<std::vector<float>>();
            branch.push_back(btag_discri.second);

            // store the discriminator values into the producer, for easy access by the analyzer
            bTagDiscriminators[btag_discri.first].push_back(btag_discri.second);

            Algorithm algo = string_to_algorithm(btag_discri.first);

            if (algo != Algorithm::UNKNOWN && BTaggingScaleFactors::has_scale_factors(algo)) {
                BTaggingScaleFactors::store_scale_factors(algo, get_flavor(jet.hadronFlavour()), {static_cast<float>(fabs(jet.eta())), static_cast<float>(jet.pt()), btag_discri.second});
            }
        }
    }
}
