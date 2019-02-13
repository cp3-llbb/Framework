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

    //! Base class for Framework producers
    /*!
     *  A producer is a simple module *producing* quantities from an event. A lot of producers are included in the framework, one for each
     *  high-level object (jets, muons, electrons, ...). These producers usually only convert CMS PAT objet into a plain ROOT tree representation.
     *
     *  Machinery for IDs and scale-factors is also included.
     *
     *  When creating a new producer, you must inherit from this pure-virtual class, and implement the @ref produce method. You'll also need to register your producer into the plugin factory. See content of the file Producers.cc for an example of how to do so.
     */
    class Producer {
        friend class ::ExTreeMaker;

        public:
            //! Base constructor
            /*!
             * @param name The name of the producer
             * @param tree_ Abstraction of the output tree. Use this to create new branches
             * @param config A link to the python configuration
             *
             * @sa ROOT::TreeGroup / ROOT::TreeWrapper documentation: http://blinkseb.github.io/TreeWrapper/#ROOT::TreeWrapper/ROOT::TreeWrapper
             * @sa tree
             */
            Producer(const std::string& name, const ROOT::TreeGroup& tree_, const edm::ParameterSet& config):
                m_name(name),
                tree(tree_),
                m_systematics(config.getUntrackedParameter<bool>("systematics", false)) {
                }

            virtual ~Producer() = default;

            //! Main method of the producer, called for each event.
            /*!
             * You have direct access to the event via the CMSSW interface with the @p event and @p setup parameters.
             *
             * @param event The CMSSW event
             * @param setup The CMSSW event setup
             * @sa https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCMSSWFramework#EdM
             * @sa CMSSW reference manual: https://cmssdt.cern.ch/SDT/doxygen/
             */
            virtual void produce(edm::Event& event, const edm::EventSetup& setup) = 0;
            //! Hook for the CMSSW consumes interface
            /*!
             * Override this method to register your tokens into the CMSSW framework via the @p collector interface
             *
             * @param pset A link to the python configuration
             * @param collector The consumes collector. Use it to register your tokens.
             *
             * @sa https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideEDMGetDataFromEvent
             */
            virtual void doConsumes(const edm::ParameterSet& pset, edm::ConsumesCollector&& collector) {}

            //! Called once at the beginning of the job
            /*!
             * @param manager The MetadataManager of the framework. Use this to store metadata to the output file
             */
            virtual void beginJob(MetadataManager& manager) {}
            //! Called once at the end of the job
            /*!
             * @param manager The MetadataManager of the framework. Use this to store metadata to the output file
             */
            virtual void endJob(MetadataManager& manager) {}

            //! Called at the beginning of a new run
            /*!
             * You have direct access to the run via the CMSSW interface with the @p run and @p setup parameters.
             *
             * @param run The CMSSW run
             * @param setup The CMSSW event setup
             */
            virtual void beginRun(const edm::Run& run, const edm::EventSetup& setup) {}
            //! Called at the end of a run
            /*!
             * You have direct access to the run via the CMSSW interface with the @p run and @p setup parameters.
             *
             * @param run The CMSSW run
             * @param setup The CMSSW event setup
             */
            virtual void endRun(const edm::Run& run, const edm::EventSetup& setup) {}

            //! Called at the beginning of a new luminosity section
            /*!
             * You have direct access to the luminosity block via the CMSSW interface with the @p lumi and @p setup parameters.
             *
             * @param run The CMSSW luminosity block
             * @param setup The CMSSW event setup
             */
            virtual void beginLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& setup) {}
            //! Called at the end of a luminosity section
            //
            /*!
             * You have direct access to the luminosity block via the CMSSW interface with the @p lumi and @p setup parameters.
             *
             * @param run The CMSSW luminosity block
             * @param setup The CMSSW event setup
             */
            virtual void endLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& setup) {}

            // Disable copy of producer
            Producer(const Producer&) = delete;
            Producer& operator=(const Producer&) = delete;

        protected:
            std::string m_name;

            //! Access point to output tree
            /*!
             * Use this variable to create new branches.
             *
             * @sa http://blinkseb.github.io/TreeWrapper/#ROOT::TreeWrapper/ROOT::TreeWrapper
             */
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

typedef edmplugin::PluginFactory<Framework::Producer* (const std::string&, const ROOT::TreeGroup&, const edm::ParameterSet&)> ExTreeMakerProducerFactory;
