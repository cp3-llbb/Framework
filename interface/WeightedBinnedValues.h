#pragma once

#include <cp3_llbb/Framework/interface/BinnedValues.h>

#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <memory>
#include <random>
#include <vector>

class WeightedBinnedValues: public BinnedValues {
    public:
        WeightedBinnedValues(const std::vector<edm::ParameterSet>& parts);
        /**
         * Randomly select one set of efficiencies from the ones
         * available, according to the fraction of integrated luminosity used to
         * compute the efficiency. A set of efficiencies evaluated on more luminosity
         * will be sampled more than one evaluated on less
         */
        virtual std::vector<float> get(const Parameters&) const override;

    private:
        mutable std::mt19937 random_generator;
        std::unique_ptr<std::discrete_distribution<int>> probability_distribution;
        std::vector<BinnedValues> efficiencies;
};
