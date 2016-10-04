#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include "TLorentzVector.h"

void MuonsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {
    edm::Handle<std::vector<pat::Muon>> muons;
    event.getByToken(m_leptons_token, muons);

    edm::Handle<double> rho_handle;
    event.getByToken(m_rho_token, rho_handle);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    const reco::Vertex& primary_vertex = (*vertices_handle)[0];

    double rho = *rho_handle;

    for (auto muon: *muons) {
        float rochester_corr_ = 1.0;
        float kamuca_corr_ = 1.0;
        if(applyRochester){
            rochester_corr_ /= muon.pt();
            float qter = 1.0;
            TLorentzVector TLmu(muon.px(), muon.py(), muon.pz(), muon.energy());
            if (event.isRealData()) {
                rmcor->momcor_data(TLmu, muon.charge(), 0, qter);
            } else {
                int ntrk = 0;
                if (!muon.innerTrack().isNull()) {
                    ntrk = muon.innerTrack()->hitPattern().trackerLayersWithMeasurement();
                }
                rmcor->momcor_mc(TLmu, muon.charge(), ntrk, qter);
            }
            muon.setP4(math::XYZTLorentzVector(TLmu.Px(), TLmu.Py(), TLmu.Pz(), TLmu.E()));
            rochester_corr_ *= muon.pt();
        }
        if (applyKaMuCa) {
            kamuca_corr_ /= muon.pt();
            muon.setP4(muon.p4() * kamucacor->getCorrectedPt(muon.pt(), muon.eta(), muon.phi(), muon.charge()) / muon.pt());
            if (!event.isRealData())
                muon.setP4(muon.p4() * kamucacor->smear(muon.pt(), muon.eta()) / muon.pt());
            kamuca_corr_ *= muon.pt();
        }
        if (! pass_cut(muon))
            continue;
        fill_candidate(muon, muon.genParticle());
        reco::MuonPFIsolation pfIso = muon.pfIsolationR03();
        computeIsolations_R03(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, muon.pt(), muon.eta(), rho);

        pfIso = muon.pfIsolationR04();
        computeIsolations_R04(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, muon.pt(), muon.eta(), rho);

        isLoose.push_back(muon.isLooseMuon());
        isMedium.push_back(muon.isMediumMuon());
        isSoft.push_back(muon.isSoftMuon(primary_vertex));
        isTight.push_back(muon.isTightMuon(primary_vertex));
        isHighPt.push_back(muon.isHighPtMuon(primary_vertex));

        // Same values used for cut-based muon ID. See:
        //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/DataFormats/MuonReco/src/MuonSelectors.cc#L756
        dxy.push_back(muon.muonBestTrack()->dxy(primary_vertex.position()));
        dz.push_back(muon.muonBestTrack()->dz(primary_vertex.position()));
        dca.push_back(muon.dB(pat::Muon::PV3D)/muon.edB(pat::Muon::PV3D));
        rochester_correction.push_back(rochester_corr_);
        kamuca_correction.push_back(kamuca_corr_);

        Parameters p {{BinningVariable::Eta, muon.eta()}, {BinningVariable::Pt, muon.pt()}};
        ScaleFactors::store_scale_factors(p, event.isRealData());
    }
}
