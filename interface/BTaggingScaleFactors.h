#pragma once

#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include <FWCore/Utilities/interface/InputTag.h>
#include <DataFormats/Common/interface/ValueMap.h>
#include <FWCore/Utilities/interface/EDMException.h>

#include <cp3_llbb/Framework/interface/Histogram.h>
#include <cp3_llbb/Framework/interface/BinnedValues.h>
#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

#include <boost/property_tree/ptree.hpp>

#include <array>
#include <memory>
#include <tuple>

enum class Algorithm {
    UNKNOWN = -1,
    CSV = 0,
    CSVv2,
    JP
};

enum class Flavor {
    B = 0,
    C = 1,
    LIGHT = 2
};

enum class SystFlavor {
    HEAVY = 1,
    LIGHT = 2
};

class BTaggingScaleFactors {

    public:
        typedef std::tuple<Algorithm, SystFlavor, std::string> branch_key_type;
        typedef std::tuple<Algorithm, Flavor, std::string> sf_key_type;

        static std::array<SystFlavor, 2> SystFlavors;

        BTaggingScaleFactors(ROOT::TreeGroup& tree):
            m_tree(tree) {
                // Empty
            }

        virtual void create_branches(const edm::ParameterSet&) final;

        virtual void store_scale_factors(Algorithm algo, Flavor flavor, const Parameters&, bool isData) final;

        virtual bool has_scale_factors(Algorithm algo) final {
            return m_algos.count(algo) != 0;
        }

    protected:
        virtual float get_scale_factor(Algorithm algo, Flavor flavor, const std::string& wp, size_t index, Variation variation = Variation::Nominal) final;

    private:
        ROOT::TreeGroup& m_tree;

        std::map<branch_key_type, std::vector<std::vector<float>>*> m_branches;
        std::map<sf_key_type, std::unique_ptr<BinnedValues>> m_scale_factors;

        std::map<Algorithm, std::vector<std::string>> m_algos;

    public:
        static inline std::string algorithm_to_string(Algorithm algo) {
            switch (algo) {
                case Algorithm::CSV:
                    return "csv";

                case Algorithm::CSVv2:
                    return "csvv2";

                case Algorithm::JP:
                    return "jp";

                default:
                    return "unknown";
            }
        }

        static inline Algorithm string_to_algorithm(const std::string& algo) {
            if ((algo == "csv") || (algo == "combinedSecondaryVertexBJetTags"))
                return Algorithm::CSV;

            if ((algo == "csvv2") || (algo == "pfCombinedInclusiveSecondaryVertexV2BJetTags"))
                return Algorithm::CSVv2;

            if ((algo == "jp") || (algo == "pfJetProbabilityBJetTags"))
                return Algorithm::JP;

            return Algorithm::UNKNOWN;
        }

        static inline std::string flavor_to_string(Flavor flavor) {
            switch (flavor) {
                case Flavor::B:
                    return "bjets";

                case Flavor::C:
                    return "cjets";

                case Flavor::LIGHT:
                    return "lightjets";
            }

            throw edm::Exception(edm::errors::NotFound, "Unsupported flavor");
        }

        static inline SystFlavor flavor_to_syst_flavor(Flavor flavor) {
            switch (flavor) {
                case Flavor::B:
                case Flavor::C:
                    return SystFlavor::HEAVY;

                case Flavor::LIGHT:
                    return SystFlavor::LIGHT;
            }

            throw edm::Exception(edm::errors::NotFound, "Unsupported flavor");
        }

        static inline std::string syst_flavor_to_string(SystFlavor flavor) {
            switch (flavor) {
                case SystFlavor::HEAVY:
                    return "heavyjet";

                case SystFlavor::LIGHT:
                    return "lightjet";
            }

            throw edm::Exception(edm::errors::NotFound, "Unsupported syst flavor");
        }

        static inline Flavor string_to_flavor(const std::string& flavor) {
            if (flavor == "bjets")
                return Flavor::B;
            else if (flavor == "cjets")
                return Flavor::C;
            else if (flavor == "lightjets")
                return Flavor::LIGHT;

            throw edm::Exception(edm::errors::NotFound, "Unsupported flavor: " + flavor);
        }

        static inline Flavor get_flavor(int hadron_flavor) {
            switch (hadron_flavor) {
                case 5:
                    return Flavor::B;

                case 4:
                    return Flavor::C;

                default:
                    return Flavor::LIGHT;
            }
        }
};
