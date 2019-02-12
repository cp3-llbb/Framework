#include <cp3_llbb/Framework/interface/EventProducer.h>

void EventProducer::produce(edm::Event& event_, const edm::EventSetup& eventSetup) {
    run = event_.id().run();
    lumi = event_.id().luminosityBlock();
    event = event_.id().event();
    is_data = event_.isRealData();

    edm::Handle<double> rho_handle;
    event_.getByToken(m_rho_token, rho_handle);

    rho = *rho_handle;

    pu_weight = 1;

    // Moriond17 reminiaod validation flags
    if (event_.isRealData()) {
        edm::Handle<bool> dupECALClusters_handle;
        edm::Handle<DetIdCollection> hitsNotReplaced_handle;

        if (event_.getByToken(m_dupECALClusters_token, dupECALClusters_handle)) {
            event_.getByToken(m_hitsNotReplaced_token, hitsNotReplaced_handle);

            dupECALClusters = *dupECALClusters_handle;
            hitsNotReplacedEmpty = hitsNotReplaced_handle->empty();
        }
    }

    edm::Handle<std::vector<PileupSummaryInfo>> pu_infos;
    if (event_.getByToken(m_pu_info_token, pu_infos)) {
        for (const auto& pu_info: *pu_infos) {
            if (pu_info.getBunchCrossing() != 0)
                continue;

            npu = pu_info.getPU_NumInteractions();
            true_interactions = pu_info.getTrueNumInteractions();

            if (m_pu_reweighter.get()) {
                pu_weight = m_pu_reweighter->getWeight(true_interactions);
                pu_weight_up = m_pu_reweighter_up->getWeight(true_interactions);
                pu_weight_down = m_pu_reweighter_down->getWeight(true_interactions);
            }
        }
    }

    edm::Handle<Framework::GenInfoAndWeights> genInfoWeights;
    event_.getByToken(m_genInfoWeightsToken, genInfoWeights);
    auto w = *genInfoWeights;
    weight = w.weight;
    pt_hat = w.pt_hat;
    ht = w.ht;
    alpha_QCD = w.alpha_QCD;
    alpha_QED = w.alpha_QED;
    q_scale = w.q_scale;
    pdf_set = w.pdf_set;
    pdf_id  = w.pdf_id;
    pdf_x   = w.pdf_x;
    pdf_weight      = w.pdf_weight;
    pdf_weight_up   = w.pdf_weight_up;
    pdf_weight_down = w.pdf_weight_down;
    hdamp_weight_up   = w.hdamp_weight_up;
    hdamp_weight_down = w.hdamp_weight_down;
    initial_state = w.initial_state;
    pdf_weight_gg      = w.pdf_weight_gg;
    pdf_weight_gg_up   = w.pdf_weight_gg_up;
    pdf_weight_gg_down = w.pdf_weight_gg_down;
    pdf_weight_qq      = w.pdf_weight_qq;
    pdf_weight_qq_up   = w.pdf_weight_qq_up;
    pdf_weight_qq_down = w.pdf_weight_qq_down;
    pdf_weight_qg      = w.pdf_weight_qg;
    pdf_weight_qg_up   = w.pdf_weight_qg_up;
    pdf_weight_qg_down = w.pdf_weight_qg_down;
    n_ME_partons = w.n_ME_partons;
    n_ME_partons_filtered = w.n_ME_partons_filtered;
    lhe_originalXWGTUP = w.lhe_originalXWGTUP;
    lhe_SCALUP = w.lhe_SCALUP;
    lhe_weights = w.lhe_weights;
    scale_weights = w.scale_weights;
}
