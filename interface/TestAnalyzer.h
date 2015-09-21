#ifndef TESTANALYZER_H
#define TESTANALYZER_H

#include <cp3_llbb/Framework/interface/Analyzer.h>
#include <cp3_llbb/Framework/interface/Category.h>

#include <cp3_llbb/Framework/interface/MuonsProducer.h>

class TwoMuonsCategory: public Category {
    virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const override {
        const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
        return muons.p4.size() >= 2;
    };

    virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const override {
        return true;
    };

    virtual void register_cuts(CutManager& manager) override {
        manager.new_cut("muon_1_pt", "pt > 30");
        manager.new_cut("muon_2_pt", "pt > 10");
    };

    virtual void evaluate_cuts_pre_analyzers(CutManager& manager, const ProducersManager& producers) const override {
        const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
        if (muons.p4[0].Pt() > 30) 
            manager.pass_cut("muon_1_pt");

        if (muons.p4[1].Pt() > 10)
            manager.pass_cut("muon_2_pt");
    }
};

class TestAnalyzer: public Framework::Analyzer {
    public:
        TestAnalyzer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
            Analyzer(name, tree_, config) {

        }

        virtual void analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager&, const AnalyzersManager&, const CategoryManager&) override;

        virtual void registerCategories(CategoryManager& manager, const edm::ParameterSet& config) {
            manager.new_category<TwoMuonsCategory>("two_muons", "At least two muons category", config);
        }

    private:

};

#endif
