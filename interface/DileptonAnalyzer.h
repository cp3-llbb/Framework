#ifndef DILEPTONANALYZER_H
#define DILEPTONANALYZER_H

#include <cp3_llbb/Framework/interface/Analyzer.h>
#include <cp3_llbb/Framework/interface/Category.h>

class DileptonAnalyzer: public Framework::Analyzer {
    enum class WorkingPoint {
        LOOSE,
        TIGHT
    };

    public:
        DileptonAnalyzer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
            Analyzer(name, tree_, config) {
                m_standalone_mode = config.getUntrackedParameter<bool>("standalone", false);

                m_muons_wp = string_to_wp(config.getUntrackedParameter<std::string>("muons_wp", "loose"));
                m_electrons_wp = string_to_wp(config.getUntrackedParameter<std::string>("electrons_wp", "loose"));

                m_electron_loose_wp_name = config.getUntrackedParameter<std::string>("electrons_loose_wp_name", "cutBasedElectronID-Spring15-50ns-V1-standalone-loose");
                m_electron_tight_wp_name = config.getUntrackedParameter<std::string>("electrons_tight_wp_name", "cutBasedElectronID-Spring15-50ns-V1-standalone-tight");
        }

        virtual void analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager&, const AnalyzersManager&, const CategoryManager&) override;

        virtual void registerCategories(CategoryManager& manager, const edm::ParameterSet&) override;

        BRANCH(mumu, std::vector<LorentzVector>);
        BRANCH(elel, std::vector<LorentzVector>);
        BRANCH(elmu, std::vector<LorentzVector>);
        BRANCH(muel, std::vector<LorentzVector>);
        BRANCH(mumu_indices, std::vector<std::pair<unsigned int,unsigned int>>);
        BRANCH(elel_indices, std::vector<std::pair<unsigned int,unsigned int>>);
        BRANCH(elmu_indices, std::vector<std::pair<unsigned int,unsigned int>>);
        BRANCH(muel_indices, std::vector<std::pair<unsigned int,unsigned int>>);

    private:
        WorkingPoint string_to_wp(const std::string& wp) {
            if (wp == "loose") {
                return WorkingPoint::LOOSE;
            } else if (wp == "tight") {
                return WorkingPoint::TIGHT;
            } else {
                throw edm::Exception(edm::errors::NotFound, "Unsupported working point: " + wp);
            }
        }

        template <class T> bool passID(const T& producer, size_t index);

        bool m_standalone_mode = false;

        WorkingPoint m_muons_wp;
        WorkingPoint m_electrons_wp;

        std::string m_electron_loose_wp_name;
        std::string m_electron_tight_wp_name;
};


#endif
