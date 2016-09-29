#include <cp3_llbb/Framework/interface/JetsProducer.h>
#include <cp3_llbb/Framework/interface/Tools.h>
#include "DataFormats/Math/interface/deltaR.h"

void JetsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::Jet>> jets;
    event.getByToken(m_jets_token, jets);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    for (const auto &jet: *jets) {
        if (! pass_cut(jet))
            continue;
        fill_candidate(jet, jet.genJet());

        jecFactor.push_back(jet.jecFactor(0));
        area.push_back(jet.jetArea());
        partonFlavor.push_back(jet.partonFlavour());
        hadronFlavor.push_back(jet.hadronFlavour());
        if (computeRegression) {
            // Variables needed for 80X b-jet energy regression as of September 29th 2016
            float nPVs_ = 0.;
            for (const auto& vertex: *vertices_handle)
            {
                if (vertex.isFake()) continue;
                if (vertex.ndof() <= 4) continue;
                if (abs(vertex.z()) > 24) continue;
                if (vertex.position().Rho() > 2) continue;
                nPVs_ += 1.;
            }
            neutralHadronEnergyFraction.push_back(jet.neutralHadronEnergyFraction());
            neutralEmEnergyFraction.push_back(jet.neutralEmEnergyFraction());
            vtxMass.push_back(jet.userFloat("vtxMass"));
            vtx3DVal.push_back(jet.userFloat("vtx3DVal"));
            vtx3DSig.push_back(jet.userFloat("vtx3DSig"));
            float vtxPx = jet.userFloat("vtxPx");
            float vtxPy = jet.userFloat("vtxPy");
            float vtxPt_ = std::sqrt(std::pow(vtxPx,2) + std::pow(vtxPy,2));
            vtxPt.push_back(vtxPt_);
            vtxNtracks.push_back(jet.userFloat("vtxNtracks"));
            // some loops needed to dig some information within the jet itself
            std::vector<std::pair<float, int>> softLeptInJet; // pt, index
            float leadTrackPt_ = 0.;
            for (unsigned int idaughter = 0; idaughter < jet.numberOfDaughters(); idaughter++)
            {
                const reco::Candidate * daughter = jet.daughter(idaughter);
                if (std::abs(daughter->pdgId()) == 11 || std::abs(daughter->pdgId()) == 13)
                    softLeptInJet.push_back( std::make_pair(daughter->pt(), idaughter) );
                if (daughter->charge() != 0)
                {
                    float leadTrackPt_tmp = daughter->pt();
                    if(leadTrackPt_tmp > leadTrackPt_)
                        leadTrackPt_ = leadTrackPt_tmp;
                }
            }
            leadTrackPt.push_back(leadTrackPt_);
            float leptonPtRel_ = -1.;
            float leptonPt_ = -1.;
            float leptonDeltaR_ = -1.;
            int softLeptIdx_ = -1;
            if (softLeptInJet.size() > 0)
            {
                std::sort(softLeptInJet.begin(), softLeptInJet.end());
                softLeptIdx_ = softLeptInJet.back().second;
                const reco::Candidate *daughter = jet.daughter(softLeptIdx_);
                leptonPt_ = daughter->pt();
                leptonPtRel_ = daughter->pt() / jet.pt();
                leptonDeltaR_ = deltaR(*daughter, jet);
            }
            leptonPtRel.push_back(leptonPtRel_);
            leptonPt.push_back(leptonPt_);
            leptonDeltaR.push_back(leptonDeltaR_);

            // Regression itself
            Jet_pt = jet.pt();
            nPVs = nPVs_;
            Jet_eta = jet.eta();
            Jet_mt = jet.mt();
            Jet_leadTrackPt = leadTrackPt_;
            Jet_leptonPtRel = leptonPtRel_;
            Jet_leptonPt = leptonPt_;
            Jet_leptonDeltaR = leptonDeltaR_;
            Jet_neHEF = jet.neutralHadronEnergyFraction();
            Jet_neEmEF = jet.neutralEmEnergyFraction();
            Jet_vtxPt = vtxPt_;
            Jet_vtxMass = jet.userFloat("vtxMass");
            Jet_vtx3dL = jet.userFloat("vtx3DVal");
            Jet_vtxNtrk = jet.userFloat("vtxNtracks");
            Jet_vtx3deL = jet.userFloat("vtx3DSig");
            regPt.push_back((bjetRegressionReader->EvaluateRegression("BDT::BDTG"))[0]);
        } else {
            regPt.push_back(jet.pt());
        }

        passLooseID.push_back(Tools::Jets::passLooseId(jet));
        passTightID.push_back(Tools::Jets::passTightId(jet));
        passTightLeptonVetoID.push_back(Tools::Jets::passTightLeptonVetoId(jet));

        if (jet.hasUserFloat("pileupJetId:fullDiscriminant"))
            puJetID.push_back(jet.userFloat("pileupJetId:fullDiscriminant"));

        for (auto& it: m_btag_discriminators) {

            float btag_discriminator = jet.bDiscriminator(it.first);
            // Protect against NaN discriminant
            if (std::isnan(btag_discriminator))
                btag_discriminator = -10;

            it.second->push_back(btag_discriminator);

            Parameters p {{BinningVariable::Eta, jet.eta()}, {BinningVariable::Pt, jet.pt()}, {BinningVariable::BTagDiscri, btag_discriminator}};

            Algorithm algo = string_to_algorithm(it.first);
            if (algo != Algorithm::UNKNOWN && BTaggingScaleFactors::has_scale_factors(algo)) {
                BTaggingScaleFactors::store_scale_factors(algo, get_flavor(jet.hadronFlavour()), p, event.isRealData());
            }
        }
    }
}
