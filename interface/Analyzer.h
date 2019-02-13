#ifndef ANALYZER_H
#define ANALYZER_H

#include <FWCore/PluginManager/interface/PluginFactory.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/Run.h>
#include <FWCore/Framework/interface/LuminosityBlock.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <cp3_llbb/Framework/interface/ProducersManager.h>
#include <cp3_llbb/Framework/interface/MetadataManager.h>
#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

#include <cp3_llbb/Framework/interface/Types.h>

#include <vector>
#include <map>

class CategoryManager;
class AnalyzersManager;
class ExTreeMaker;

namespace Framework {

    class Analyzer {
        friend class ::ExTreeMaker;

        public:

            Analyzer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
                m_name(name),
                tree(tree_),
                m_systematics(config.getUntrackedParameter<bool>("systematics", false)) {
                }
            virtual ~Analyzer() = default;

            virtual void analyze(const edm::Event&, const edm::EventSetup&, const ProducersManager&, const AnalyzersManager&, const CategoryManager&) = 0;
            virtual void doConsumes(const edm::ParameterSet&, edm::ConsumesCollector&& collector) {}

            virtual void registerCategories(CategoryManager& manager, const edm::ParameterSet& config) {}

            virtual void beginJob(MetadataManager&) {}
            virtual void endJob(MetadataManager&) {}

            virtual void beginRun(const edm::Run&, const edm::EventSetup&) {}
            virtual void endRun(const edm::Run&, const edm::EventSetup&) {}

            virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}
            virtual void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}

            // Disable copy of analyzer
            Analyzer(const Analyzer&) = delete;
            Analyzer& operator=(const Analyzer&) = delete;

        protected:
            std::string m_name;

            ROOT::TreeGroup tree;

            inline bool doingSystematics() const {
                return m_systematics;
            }

        private:
            bool hasRun() const {
                return m_run;
            }

            void setRun(bool run) {
                m_run = run;
            }

            bool m_run; //< A flag indicating if the analyzer has already been run for this event

            // If true, this analyzer is producing systematics related quantities
            bool m_systematics;
    };

}

typedef edmplugin::PluginFactory<Framework::Analyzer* (const std::string&, const ROOT::TreeGroup&, const edm::ParameterSet&)> ExTreeMakerAnalyzerFactory;

#endif
