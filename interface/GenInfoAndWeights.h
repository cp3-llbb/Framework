#pragma once

#include <utility>
#include <string>
#include <vector>

namespace Framework {
/**
 * A struct for generator info (weights etc.)
 *
 * To pass these on from GenInfoEventWeightsProducer (which is run for every event)
 * to EventProducer (where the TTree is filled), which may be behind a filter.
 *
 * @see EventProducer
 * @see GenInfoEventWeightsProducer
 */
struct GenInfoAndWeights {
  float weight = 1.;
  float pt_hat, ht;
  float alpha_QCD, alpha_QED, q_scale;
  uint32_t pdf_set;
  std::pair<int,int> pdf_id;
  std::pair<float,float> pdf_x;
  float pdf_weight = 1.;
  float pdf_weight_up = 1.;
  float pdf_weight_down = 1.;
  float hdamp_weight_up = 1.;
  float hdamp_weight_down = 1.;
  uint8_t initial_state;
  float pdf_weight_gg = 1.;
  float pdf_weight_gg_up = 1.;
  float pdf_weight_gg_down = 1.;
  float pdf_weight_qq = 1.;
  float pdf_weight_qq_up = 1.;
  float pdf_weight_qq_down = 1.;
  float pdf_weight_qg = 1.;
  float pdf_weight_qg_up = 1.;
  float pdf_weight_qg_down = 1.;
  int n_ME_partons, n_ME_partons_filtered;
  float lhe_originalXWGTUP, lhe_SCALUP;
  std::vector<std::pair<std::string,float>> lhe_weights;
  std::vector<float> scale_weights;
};
}
