#include <cp3_llbb/Framework/interface/FatJetsProducer.h>
#include <cp3_llbb/Framework/interface/Tools.h>
#include <DataFormats/BTauReco/interface/CATopJetTagInfo.h>

void FatJetsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::Jet>> jets;
    event.getByToken(m_jets_token, jets);

    for (const auto& jet: *jets) {
        if (! pass_cut(jet))
            continue;

        fill_candidate(jet, jet.genJet());

        Flavor jet_flavor = get_flavor(jet.hadronFlavour());

        jecFactor.push_back(jet.jecFactor(0));
        area.push_back(jet.jetArea());
        partonFlavor.push_back(jet.partonFlavour());
        hadronFlavor.push_back(jet.hadronFlavour());
        systFlavor.push_back(static_cast<int8_t>(BTaggingScaleFactors::flavor_to_syst_flavor(jet_flavor)));

        passLooseID.push_back(Tools::Jets::passLooseId(jet));
        passTightID.push_back(Tools::Jets::passTightId(jet));
        passTightLeptonVetoID.push_back(Tools::Jets::passTightLeptonVetoId(jet));

        tau1.push_back(jet.userFloat(Njettinesstau1));
        tau2.push_back(jet.userFloat(Njettinesstau2));
        tau3.push_back(jet.userFloat(Njettinesstau3));

        // Reconstruct puppi p4
        ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<float>> puppi_p4_(
                jet.userFloat("ak8PFJetsPuppiValueMap:pt"),
                jet.userFloat("ak8PFJetsPuppiValueMap:eta"),
                jet.userFloat("ak8PFJetsPuppiValueMap:phi"),
                jet.userFloat("ak8PFJetsPuppiValueMap:mass")
                );
        puppi_p4.push_back(LorentzVector(puppi_p4_));
        puppi_tau1.push_back(jet.userFloat("ak8PFJetsPuppiValueMap:NjettinessAK8PuppiTau1"));
        puppi_tau2.push_back(jet.userFloat("ak8PFJetsPuppiValueMap:NjettinessAK8PuppiTau2"));
        puppi_tau3.push_back(jet.userFloat("ak8PFJetsPuppiValueMap:NjettinessAK8PuppiTau3"));

        softdrop_mass.push_back(jet.userFloat("ak8PFJetsCHSSoftDropMass"));
        pruned_mass.push_back(jet.userFloat("ak8PFJetsCHSPrunedMass"));

        // Subjets
        // 1) SoftDrop
        const auto& wSubjets = jet.subjets(SoftDropSubjets);
        std::map<std::string, std::vector<float>> subjets_btag_discriminators;
        std::vector<LorentzVector> subjets_p4;
        for (const auto& iw: wSubjets) {
            subjets_p4.push_back(LorentzVector(iw->pt(), iw->eta(), iw->phi(), iw->energy()));
            for (const auto& subjet_btag: m_subjets_btag_discriminators) {
                subjets_btag_discriminators[subjet_btag].push_back(iw->bDiscriminator(subjet_btag));
            }
        }

        softdrop_subjets_p4.push_back(subjets_p4);
        for (const auto& subjet_btag: m_subjets_btag_discriminators) {
            m_softdrop_btag_discriminators_branches[subjet_btag]->push_back(subjets_btag_discriminators[subjet_btag]);
        }

        // 2) Top tagger
        const auto& wPuppiSubjets = jet.subjets(SoftDropPuppiSubjets);
        subjets_p4.clear();
        subjets_btag_discriminators.clear();
        for (const auto& it: wPuppiSubjets) {
            subjets_p4.push_back(LorentzVector(it->pt(), it->eta(), it->phi(), it->energy()));
            for (const auto& subjet_btag: m_subjets_btag_discriminators) {
                subjets_btag_discriminators[subjet_btag].push_back(it->bDiscriminator(subjet_btag));
            }
        }

        softdrop_puppi_subjets_p4.push_back(subjets_p4);
        for (const auto& subjet_btag: m_subjets_btag_discriminators) {
            m_softdrop_puppi_btag_discriminators_branches[subjet_btag]->push_back(subjets_btag_discriminators[subjet_btag]);
        }

        for (auto& it: m_btag_discriminators) {

            float btag_discriminator = jet.bDiscriminator(it.first);
            // Protect against NaN discriminant
            if (std::isnan(btag_discriminator))
                btag_discriminator = -10;

            it.second->push_back(btag_discriminator);

            Parameters p {{BinningVariable::Eta, jet.eta()}, {BinningVariable::Pt, jet.pt()}, {BinningVariable::BTagDiscri, btag_discriminator}};

            Algorithm algo = string_to_algorithm(it.first);
            if (algo != Algorithm::UNKNOWN && BTaggingScaleFactors::has_scale_factors(algo)) {
                BTaggingScaleFactors::store_scale_factors(algo, jet_flavor, p, event.isRealData());
            }
        }
    }
}

float FatJetsProducer::get_scale_factor(Algorithm algo, const std::string& wp, size_t index, Variation variation/* = Variation::Nominal*/) {

    auto flavor = BTaggingScaleFactors::get_flavor(hadronFlavor[index]);

    return BTaggingScaleFactors::get_scale_factor(algo, flavor, wp, index, variation);
}
