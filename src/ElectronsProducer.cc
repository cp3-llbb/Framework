#include <cp3_llbb/Framework/interface/ElectronsProducer.h>

void ElectronsProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    Identifiable::retrieves_id_tokens(event, eventSetup);

    edm::Handle<std::vector<pat::Electron>> electrons;
    event.getByToken(m_leptons_token, electrons);

    edm::Handle<double> rho_handle;
    event.getByToken(m_rho_token, rho_handle);

    edm::Handle<std::vector<reco::Vertex>> vertices_handle;
    event.getByToken(m_vertices_token, vertices_handle);

    edm::Handle<edm::ValueMap<float>> mva_id_values_handle;
    edm::Handle<edm::ValueMap<int>> mva_id_categories_handle;

    if (! m_mva_id_values_map_token.isUninitialized()) {
        event.getByToken(m_mva_id_values_map_token, mva_id_values_handle);
        event.getByToken(m_mva_id_categories_map_token, mva_id_categories_handle);
    }

    const reco::Vertex& primary_vertex = (*vertices_handle)[0];

    double rho = *rho_handle;

    products.clear();

    size_t index = 0;
    for (const auto& electron: *electrons) {
        if (! pass_cut(electron)) {
            index++;
            continue;
        }

        fill_candidate(electron, electron.genParticle());

        reco::GsfElectron::PflowIsolationVariables pfIso = electron.pfIsolationVariables();
        computeIsolations_R03(pfIso.sumChargedHadronPt, pfIso.sumNeutralHadronEt, pfIso.sumPhotonEt, pfIso.sumPUPt, electron.pt(), electron.superCluster()->eta(), rho);
        computeIsolations_R04(electron.chargedHadronIso(), electron.neutralHadronIso(), electron.photonIso(), electron.puChargedHadronIso(), electron.pt(), electron.superCluster()->eta(), rho);

        const pat::ElectronRef electronRef(electrons, index++);
        Identifiable::produce_id(electronRef);
        products.push_back(electronRef);

        isEB.push_back(electron.isEB());
        isEE.push_back(electron.isEE());
        
        // Same values used for cut-based electron ID. See:
        //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/RecoEgamma/ElectronIdentification/plugins/cuts/GsfEleDzCut.cc#L64
        //     https://github.com/cms-sw/cmssw/blob/CMSSW_7_4_15/RecoEgamma/ElectronIdentification/plugins/cuts/GsfEleDxyCut.cc#L64
        dxy.push_back(electron.gsfTrack()->dxy(primary_vertex.position()));
        dz.push_back(electron.gsfTrack()->dz(primary_vertex.position()));
        dca.push_back(electron.dB(pat::Electron::PV3D)/electron.edB(pat::Electron::PV3D));
        ecalPFClusterIso.push_back(electron.ecalPFClusterIso());
        hcalPFClusterIso.push_back(electron.hcalPFClusterIso());
        trackIso.push_back(electron.trackIso());
        // see https://github.com/latinos/LatinoTrees/blob/cc89e18d1a956f633cf59a7143f24491716d1047/DataFormats/src/SkimEvent.cc#L5831
        expectedMissingInnerHits.push_back(electron.gsfTrack()->hitPattern().numberOfHits(reco::HitPattern::MISSING_INNER_HITS));

        // MVA id
        if (! m_mva_id_values_map_token.isUninitialized()) {
            mva_id_values.push_back((*mva_id_values_handle)[electronRef]);
            mva_id_categories.push_back((*mva_id_categories_handle)[electronRef]);
        }

        //PHASE2 ELECTRON ID :
        //Values from here :
        //https://github.com/CMS-HGCAL/EgammaTools/blob/master/ELECTRONBDT.md#recommended-id-cuts
        
        const float mvaOutput = electron.userFloat("mvaValue");
        bool passesLoose{false}, passesMedium{false}, passesTight{false};
        
        if ( !electron.isEB() ) {
          if ( electron.pt() > 20. ) {
            passesTight  = mvaOutput >  0.983;
            passesMedium = mvaOutput >  0.591;
            passesLoose  = mvaOutput > -0.919;
          }
          else if ( electron.pt() > 10. ) {
            passesTight  = mvaOutput >  0.969;
            passesMedium = mvaOutput >  0.777;
            passesLoose  = mvaOutput > -0.320; 
          }
        }
        else {
          if ( electron.pt() > 20. ) {
            passesTight  = mvaOutput >  0.988;
            passesMedium = mvaOutput >  0.723;
            passesLoose  = mvaOutput > -0.797;
          }
          else if ( electron.pt() > 10. ) {
            passesTight  = mvaOutput >  0.986;
            passesMedium = mvaOutput >  0.855;
            passesLoose  = mvaOutput > -0.661;
          }
        }

        phase2_mva_id_values.push_back(mvaOutput);
        phase2_mva_id_loose.push_back(passesLoose);
        phase2_mva_id_medium.push_back(passesMedium);
        phase2_mva_id_tight.push_back(passesTight);


        Parameters p {{BinningVariable::Eta, electron.superCluster()->eta()}, {BinningVariable::Pt, electron.pt()}};
        ScaleFactors::store_scale_factors(p, event.isRealData());
    }
    Identifiable::clean();
}
