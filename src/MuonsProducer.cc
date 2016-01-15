#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/MuonsProducer.h>
#include <cp3_llbb/Framework/interface/rochcor2015.h>
#include "TLorentzVector.h"
#include "TMath.h"
void MuonsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<std::vector<pat::Muon>> muons;
    event.getByToken(m_leptons_token, muons);

    edm::Handle<double> rho_handle;
    event.getByToken(m_rho_token, rho_handle);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    const reco::Vertex& primary_vertex = (*vertices_handle)[0];

    double rho = *rho_handle;
    rochcor2015 *rmcor = new rochcor2015();
    for (const auto& muon: *muons) {
	float qter = 1.0;
        pat::Muon muoncorr=muon;
        TLorentzVector TLmu;
        TLmu.SetPxPyPzE(muon.px(),muon.py(),muon.pz(),muon.energy());
        if(event.isRealData()){
                rmcor->momcor_data(TLmu, muon.charge(), 0, qter);
        }
        else{
                rmcor->momcor_mc(TLmu, muon.charge(), 0, qter);
        }
	math::XYZTLorentzVector lv(TLmu.Px(),TLmu.Py(),TLmu.Pz(),TLmu.E());
	muoncorr.setP4(lv);

        if (! pass_cut(muoncorr))
            continue;

        fill_candidate(muoncorr, muon.genParticle());

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
        ScaleFactors::store_scale_factors({static_cast<float>(fabs(muon.eta())), static_cast<float>(muon.pt())},event.isRealData());
    }
}
