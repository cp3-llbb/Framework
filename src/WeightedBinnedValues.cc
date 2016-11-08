#include <cp3_llbb/Framework/interface/WeightedBinnedValues.h>

#include <cp3_llbb/Framework/interface/BinnedValuesJSONParser.h>

WeightedBinnedValues::WeightedBinnedValues(const std::vector<edm::ParameterSet>& parts):
    random_generator(42) {

    std::vector<double> weights;

    for (const auto& p: parts) {
        double weight = p.getUntrackedParameter<double>("weight");
        weights.push_back(weight);

        BinnedValuesJSONParser parser(p.getUntrackedParameter<edm::FileInPath>("file").fullPath());
        efficiencies.push_back(std::move(parser.get_values()));
    }

    probability_distribution.reset(new std::discrete_distribution<>(weights.begin(), weights.end()));
}

std::vector<float> WeightedBinnedValues::get(const Parameters& parameters) const {
    return efficiencies[(*probability_distribution)(random_generator)].get(parameters);
}
