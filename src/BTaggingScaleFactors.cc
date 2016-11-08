#include <cp3_llbb/Framework/interface/BTaggingScaleFactors.h>
#include <cp3_llbb/Framework/interface/BinnedValuesJSONParser.h>
#include <cp3_llbb/Framework/interface/WeightedBinnedValues.h>

#include <iostream>

//#define SF_DEBUG

std::array<SystFlavor, 2> BTaggingScaleFactors::SystFlavors = {{SystFlavor::HEAVY, SystFlavor::LIGHT}};

void BTaggingScaleFactors::create_branches(const edm::ParameterSet& config) {

    if (config.existsAs<edm::ParameterSet>("scale_factors", false)) {
#ifdef SF_DEBUG
        std::cout << "B-tagging scale factors: " << std::endl;
#endif
        const edm::ParameterSet& scale_factors = config.getUntrackedParameter<edm::ParameterSet>("scale_factors");
        std::vector<std::string> scale_factors_name = scale_factors.getParameterNames();

        for (const std::string& scale_factor: scale_factors_name) {

            const edm::ParameterSet& scale_factor_set = scale_factors.getUntrackedParameterSet(scale_factor);

            std::string algo_str = scale_factor_set.getUntrackedParameter<std::string>("algorithm");
            Algorithm algo =  string_to_algorithm(algo_str);
            if (algo == Algorithm::UNKNOWN) {
#ifdef SF_DEBUG
                std::cout << "\tUnsupported b-tagging algorithm: " << algo_str << std::endl;
#endif
                continue;
            }

            std::string wp = scale_factor_set.getUntrackedParameter<std::string>("working_point");

            m_algos[algo].push_back(wp);

            std::vector<edm::ParameterSet> files = scale_factor_set.getUntrackedParameter<std::vector<edm::ParameterSet>>("files");

            for (auto syst_flavor: SystFlavors) {
                std::string branch_name = "sf_" + algo_str + "_" + syst_flavor_to_string(syst_flavor) + "_" + wp;
                branch_key_type branch_key = std::make_tuple(algo, syst_flavor, wp);
                m_branches.emplace(branch_key, & m_tree[branch_name].write<std::vector<std::vector<float>>>());
            }

            for (auto& file_set: files) {
                std::string flavor = file_set.getUntrackedParameter<std::string>("flavor");

                std::cout << "    Registering new scale-factor for algo: " << algo_str << "  wp: " << wp << "  flavor: " << flavor;

                sf_key_type sf_key = std::make_tuple(algo, string_to_flavor(flavor), wp);

                // The value can be either a FileInPath for a standard JSON file, or a vector
                // of ParameterSet for weighted values
                if (file_set.existsAs<edm::FileInPath>("file", false)) {
                    std::string file = file_set.getUntrackedParameter<edm::FileInPath>("file").fullPath();
                    std::cout << " -> non-weighted." << std::endl;

                    BinnedValuesJSONParser parser(file);
                    m_scale_factors.emplace(sf_key, std::unique_ptr<BinnedValues>(new BinnedValues(std::move(parser.get_values()))));
                } else {
                    const auto& parts = file_set.getUntrackedParameter<std::vector<edm::ParameterSet>>("file");
                    std::unique_ptr<BinnedValues> values(new WeightedBinnedValues(parts));
                    m_scale_factors.emplace(sf_key, std::move(values));
                    std::cout << " -> weighted (" << parts.size() << " components)." << std::endl;
                }
            }
        }
#ifdef SF_DEBUG
        std::cout << std::endl;
#endif
    }

}

void BTaggingScaleFactors::store_scale_factors(Algorithm algo_, Flavor flavor, const Parameters& parameters, bool isData) {

    auto algo_it = m_algos.find(algo_);
    if (algo_it == m_algos.end())
        throw edm::Exception(edm::errors::NotFound, "No scale factors for this algorithm. Please check your python configuration.");

    SystFlavor jet_syst_flavor = flavor_to_syst_flavor(flavor);
    for (const auto& wp: algo_it->second) {
        sf_key_type sf_key = std::make_tuple(algo_, flavor, wp);
        for (auto syst_flavor: SystFlavors) {
            branch_key_type branch_key = std::make_tuple(algo_, syst_flavor, wp);

            // Store a dummy SF for data or if the jet flavor is not the right one
            if (isData || syst_flavor != jet_syst_flavor)
                (*m_branches[branch_key]).push_back({1., 0., 0.});
            else
                (*m_branches[branch_key]).push_back(m_scale_factors.at(sf_key)->get(parameters));
        }
    }
}

float BTaggingScaleFactors::get_scale_factor(Algorithm algo, Flavor flavor, const std::string& wp, size_t index, Variation variation/* = Variation::Nominal*/) {

    branch_key_type key = std::make_tuple(algo, flavor_to_syst_flavor(flavor), wp);
    auto sf = m_branches.find(key);

    if (sf == m_branches.end())
        return 0;

    if (index >= sf->second->size())
        return 0;

    return (*sf->second)[index][static_cast<size_t>(variation)];
}
