#pragma once

#include <FWCore/PluginManager/interface/PluginFactory.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/Run.h>
#include <FWCore/Framework/interface/LuminosityBlock.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>
#include <cp3_llbb/Framework/interface/MetadataManager.h>

#include <cp3_llbb/Framework/interface/Types.h>

#include <vector>
#include <map>

class ExTreeMaker;

namespace Framework {

    class Producer {
        friend class ::ExTreeMaker;

        public:
            Producer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
                m_name(name),
                tree(tree_) {
                }

            virtual void produce(edm::Event&, const edm::EventSetup&) = 0;
            virtual void doConsumes(const edm::ParameterSet&, edm::ConsumesCollector&& collector) {}

            virtual void beginJob(MetadataManager&) {}
            virtual void endJob(MetadataManager&) {}

            virtual void beginRun(const edm::Run&, const edm::EventSetup&) {}
            virtual void endRun(const edm::Run&, const edm::EventSetup&) {}

            virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}
            virtual void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}

        protected:
            std::string m_name;
            ROOT::TreeGroup tree;

        private:
            bool hasRun() const {
                return m_run;
            }

            void setRun(bool run) {
                m_run = run;
            }

            bool m_run; //< A flag indicating if the analyzer has already been run for this event
    };

}

typedef edmplugin::PluginFactory<Framework::Producer* (const std::string&, const ROOT::TreeGroup&, const edm::ParameterSet&)> ExTreeMakerProducerFactory;
