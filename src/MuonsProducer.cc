#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include "TLorentzVector.h"

bool isMediumMuon2016(const reco::Muon & recoMu) {
    bool goodGlob = recoMu.isGlobalMuon() &&
        recoMu.globalTrack()->normalizedChi2() < 3 &&
        recoMu.combinedQuality().chi2LocalPosition < 12 &&
        recoMu.combinedQuality().trkKink < 20;

    bool isMedium = muon::isLooseMuon(recoMu) &&
        recoMu.innerTrack()->validFraction() > 0.49 &&
        muon::segmentCompatibility(recoMu) > (goodGlob ? 0.303 : 0.451);

    return isMedium;
}

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
        if (! pass_cut(muon))
            continue;
        fill_candidate(muon, muon.genParticle());
        reco::MuonPFIsolation pfIso = muon.pfIsolationR03();
        computeIsolations_R03(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, muon.pt(), muon.eta(), rho);

        pfIso = muon.pfIsolationR04();
        computeIsolations_R04(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, muon.pt(), muon.eta(), rho);
        
        isLoose.push_back(muon.isLooseMuon());
        isMedium.push_back(muon.isMediumMuon());
        isMedium2016.push_back(isMediumMuon2016(muon));
        isSoft.push_back(muon.isSoftMuon(primary_vertex));
        isTight.push_back(muon.isTightMuon(primary_vertex));
        isHighPt.push_back(muon.isHighPtMuon(primary_vertex));

        // Same values used for cut-based muon ID. See:
        //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/DataFormats/MuonReco/src/MuonSelectors.cc#L756
        dxy.push_back(muon.muonBestTrack()->dxy(primary_vertex.position()));
        dz.push_back(muon.muonBestTrack()->dz(primary_vertex.position()));
        dca.push_back(muon.dB(pat::Muon::PV3D)/muon.edB(pat::Muon::PV3D));

        Parameters p {{BinningVariable::Eta, muon.eta()}, {BinningVariable::Pt, muon.pt()}};
        ScaleFactors::store_scale_factors(p, event.isRealData());
    }
}
