#ifndef BTAGSANALYZER_H
#define BTAGSANALYZER_H

#include <cp3_llbb/Framework/interface/Analyzer.h>

class BTagsAnalyzer: public Framework::Analyzer {

  public:
        BTagsAnalyzer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
            Analyzer(name, tree_, config),
            m_discr_cut(config.getUntrackedParameter<double>("discr_cut")), 
            m_eta_cut(config.getUntrackedParameter<double>("eta_cut")), 
            m_pt_cut(config.getUntrackedParameter<double>("pt_cut")), 
            m_discr_name(config.getUntrackedParameter<std::string>("discr_name")) 
            { }

        virtual void analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager&, const AnalyzersManager&, const CategoryManager&) override;

        BRANCH(indices, std::vector<uint8_t>);

  private:

        const float m_discr_cut;
        const float m_eta_cut;
        const float m_pt_cut;
        const std::string m_discr_name;
};


#endif
