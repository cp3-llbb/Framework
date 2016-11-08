#pragma once

#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <DataFormats/Common/interface/ValueMap.h>

#include <cp3_llbb/Framework/interface/Histogram.h>
#include <cp3_llbb/Framework/interface/BinnedValues.h>
#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

#include <boost/property_tree/ptree.hpp>

#include <memory>

class ScaleFactors {

    public:
        ScaleFactors(ROOT::TreeGroup& tree):
            m_tree(tree) {
                // Empty
            }

        virtual void create_branches(const edm::ParameterSet&) final;
        virtual void create_branch(const std::string& scale_factor, const std::string& branch_name);

        virtual void store_scale_factors(const Parameters&, bool isData) final;

        virtual float get_scale_factor(const std::string& tag, size_t index, Variation variation = Variation::Nominal) final;

    private:
        ROOT::TreeGroup& m_tree;

        std::map<std::string, std::vector<std::vector<float>>*> m_branches;
        std::map<std::string, std::unique_ptr<BinnedValues>> m_scale_factors;
};
