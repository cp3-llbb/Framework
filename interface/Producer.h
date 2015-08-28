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
#include <cp3_llbb/Framework/interface/Tools.h>
#include <cp3_llbb/Framework/interface/MetadataManager.h>

#include <Math/Vector4D.h>

#include <boost/any.hpp>

#include <vector>
#include <map>

typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiE4D<float>> LorentzVector;

namespace Framework {

    class Producer {
        public:
            Producer(const std::string& name, ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
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

            template<typename T> const T& get(int i) const { return boost::any_cast<const T&>(tree[variable]); }
            
            template<typename T> void push_back(std::string variable, T value){ 
                std::vector<T> &var = boost::any_cast<std::vector<T>&>(tree[variable]);
                var.push_back(value);
                _data.push_back(std::make_pair(variable, value));
            }

            const boost::any& operator[](const int index) const { return data[index]; }
            
        protected:
            std::string m_name;
            ROOT::TreeGroup tree;
            
            std::map<std::string, boost::any&> treeData;
            std::vector<std::map<std::string, boost::any>> data;
    };

}

typedef edmplugin::PluginFactory<Framework::Producer* (const std::string&, const ROOT::TreeGroup&, const edm::ParameterSet&)> ExTreeMakerProducerFactory;
