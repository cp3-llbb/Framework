#include <DataFormats/PatCandidates/interface/PackedCandidate.h>
#include "DataFormats/Math/interface/Vector3D.h"
#include <cp3_llbb/Framework/interface/TausProducer.h>
#include "TLorentzVector.h"


void TausProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {
    edm::Handle<std::vector<pat::Tau>> taus;
    event.getByToken(m_leptons_token, taus);

    edm::Handle<double> rho_handle;
    event.getByToken(m_rho_token, rho_handle);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    const reco::Vertex& primary_vertex = (*vertices_handle)[0];

    double rho = *rho_handle;

    for (auto tau: *taus) {
        // if (! pass_cut(tau))
        //     continue;
        fill_candidate(tau, tau.genParticle());
        // reco::MuonPFIsolation pfIso = tau.pfIsolationR03();
        // computeIsolations_R03(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, tau.pt(), tau.eta(), rho);
        //
        // pfIso = tau.pfIsolationR04();
        // computeIsolations_R04(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, tau.pt(), tau.eta(), rho);
        //
        decayModeFinding.push_back(tau.tauID("decayModeFinding"));
        decayModeFindingNewDMs.push_back(tau.tauID("decayModeFindingNewDMs"));
        byLooseCombinedIsolationDeltaBetaCorr3Hits.push_back(tau.tauID("byLooseCombinedIsolationDeltaBetaCorr3Hits"));
        byMediumCombinedIsolationDeltaBetaCorr3Hits.push_back(tau.tauID("byMediumCombinedIsolationDeltaBetaCorr3Hits"));
        byTightCombinedIsolationDeltaBetaCorr3Hits.push_back(tau.tauID("byTightCombinedIsolationDeltaBetaCorr3Hits"));

        byVLooseIsolationMVArun2v1DBoldDMwLT.push_back(tau.tauID("byVLooseIsolationMVArun2v1DBoldDMwLT"));
        byLooseIsolationMVArun2v1DBoldDMwLT.push_back(tau.tauID("byLooseIsolationMVArun2v1DBoldDMwLT"));
        byMediumIsolationMVArun2v1DBoldDMwLT.push_back(tau.tauID("byMediumIsolationMVArun2v1DBoldDMwLT"));
        byTightIsolationMVArun2v1DBoldDMwLT.push_back(tau.tauID("byTightIsolationMVArun2v1DBoldDMwLT"));
        byVTightIsolationMVArun2v1DBoldDMwLT.push_back(tau.tauID("byVTightIsolationMVArun2v1DBoldDMwLT"));
        byVLooseIsolationMVArun2v1DBdR03oldDMwLT.push_back(tau.tauID("byVLooseIsolationMVArun2v1DBdR03oldDMwLT"));
        byLooseIsolationMVArun2v1DBdR03oldDMwLT.push_back(tau.tauID("byLooseIsolationMVArun2v1DBdR03oldDMwLT"));
        byMediumIsolationMVArun2v1DBdR03oldDMwLT.push_back(tau.tauID("byMediumIsolationMVArun2v1DBdR03oldDMwLT"));
        byTightIsolationMVArun2v1DBdR03oldDMwLT.push_back(tau.tauID("byTightIsolationMVArun2v1DBdR03oldDMwLT"));
        byVTightIsolationMVArun2v1DBdR03oldDMwLT.push_back(tau.tauID("byVTightIsolationMVArun2v1DBdR03oldDMwLT"));
        byVLooseIsolationMVArun2v1DBnewDMwLT.push_back(tau.tauID("byVLooseIsolationMVArun2v1DBnewDMwLT"));
        byLooseIsolationMVArun2v1DBnewDMwLT.push_back(tau.tauID("byLooseIsolationMVArun2v1DBnewDMwLT"));
        byMediumIsolationMVArun2v1DBnewDMwLT.push_back(tau.tauID("byMediumIsolationMVArun2v1DBnewDMwLT"));
        byTightIsolationMVArun2v1DBnewDMwLT.push_back(tau.tauID("byTightIsolationMVArun2v1DBnewDMwLT"));
        byVTightIsolationMVArun2v1DBnewDMwLT.push_back(tau.tauID("byVTightIsolationMVArun2v1DBnewDMwLT"));
        againstMuonLoose3.push_back(tau.tauID("againstMuonLoose3"));
        againstMuonTight3.push_back(tau.tauID("againstMuonTight3"));
        againstElectronVLooseMVA6.push_back(tau.tauID("againstElectronVLooseMVA6"));
        againstElectronLooseMVA6.push_back(tau.tauID("againstElectronLooseMVA6"));
        againstElectronMediumMVA6.push_back(tau.tauID("againstElectronMediumMVA6"));
        againstElectronTightMVA6.push_back(tau.tauID("againstElectronTightMVA6"));
        againstElectronVTightMVA6.push_back(tau.tauID("againstElectronVTightMVA6"));
        decayMode.push_back(tau.decayMode());

        // // Same values used for cut-based tau ID. See:
        // //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/DataFormats/MuonReco/src/MuonSelectors.cc#L756
        // dxy.push_back(tau.muonBestTrack()->dxy(primary_vertex.position()));
        // dz.push_back(tau.muonBestTrack()->dz(primary_vertex.position()));
        // dca.push_back(tau.dB(pat::tau::PV3D)/tau.edB(pat::tau::PV3D));
        //
        Parameters p {{BinningVariable::Eta, tau.eta()}, {BinningVariable::Pt, tau.pt()}};
        ScaleFactors::store_scale_factors(p, event.isRealData());
    }
}
