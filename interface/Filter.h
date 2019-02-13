#pragma once

#include <FWCore/PluginManager/interface/PluginFactory.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/Run.h>
#include <FWCore/Framework/interface/LuminosityBlock.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <cp3_llbb/Framework/interface/Types.h>
#include <cp3_llbb/Framework/interface/MetadataManager.h>

#include <Math/Vector4D.h>

#include <vector>
#include <map>

namespace Framework {

    class Filter {
        public:
            Filter(const std::string& name, const edm::ParameterSet& config):
                m_name(name) {
                }
            virtual ~Filter() = default;

            virtual bool filter(edm::Event&, const edm::EventSetup&) = 0;
            virtual void doConsumes(const edm::ParameterSet&, edm::ConsumesCollector&& collector) {}

            virtual void beginJob(MetadataManager&) {}
            virtual void endJob(MetadataManager&) {}

            virtual void beginRun(const edm::Run&, const edm::EventSetup&) {}
            virtual void endRun(const edm::Run&, const edm::EventSetup&) {}

            virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}
            virtual void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) {}

        protected:
            std::string m_name;
    };

}

typedef edmplugin::PluginFactory<Framework::Filter* (const std::string&, const edm::ParameterSet&)> ExTreeMakerFilterFactory;
