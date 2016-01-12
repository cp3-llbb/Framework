#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include <FWCore/Framework/interface/ConsumesCollector.h>
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "cp3_llbb/Framework/interface/Analyzer.h"
#include "cp3_llbb/Framework/interface/Producer.h"
#include "cp3_llbb/Framework/interface/Filter.h"
#include "cp3_llbb/Framework/interface/Category.h"
#include "cp3_llbb/Framework/interface/ProducerGetter.h"
#include "cp3_llbb/Framework/interface/ProducersManager.h"
#include "cp3_llbb/Framework/interface/AnalyzerGetter.h"
#include "cp3_llbb/Framework/interface/AnalyzersManager.h"

class ExTreeMaker: public edm::EDProducer, ProducerGetter, AnalyzerGetter {
    friend class ProducersManager;
    friend class AnalyzersManager;

    struct AnalyzerWrapper {
        std::shared_ptr<Framework::Analyzer> analyzer;
        std::string name;
        std::string prefix;
    };

    public:
        explicit ExTreeMaker(const edm::ParameterSet&);
        ~ExTreeMaker();

    private:
        virtual void beginJob() override;
        virtual void produce(edm::Event&, const edm::EventSetup&) override;
        virtual void endJob() override;

        virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
        virtual void endRun(const edm::Run&, const edm::EventSetup&) override;

        virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) override;
        virtual void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&) override;

        // From ProducerGetter
        virtual const Framework::Producer& getProducer(const std::string& name) const override;
        virtual bool producerExists(const std::string& name) const override;
        std::unique_ptr<ProducersManager> m_producers_manager;

        // From AnalyzerGetter
        virtual const Framework::Analyzer& getAnalyzer(const std::string& name) const override;
        virtual bool analyzerExists(const std::string& name) const override;
        std::unique_ptr<AnalyzersManager> m_analyzers_manager;

        std::string m_output_filename;
        std::unique_ptr<TFile> m_output;
        std::unique_ptr<ROOT::TreeWrapper> m_wrapper;

        std::unordered_map<std::string, std::shared_ptr<Framework::Filter>> m_filters;

        // Order is important, we can't use a map here
        std::vector<std::pair<std::string, std::shared_ptr<Framework::Producer>>> m_producers;
        std::vector<AnalyzerWrapper> m_analyzers;
        std::vector<std::string> m_analyzers_name;

        // Categories
        std::unique_ptr<CategoryManager> m_categories;

        // Metadata
        std::unique_ptr<MetadataManager> m_metadata;

        // Timing
        typedef std::chrono::system_clock clock;
        typedef std::chrono::milliseconds ms;
        typedef std::chrono::seconds seconds;

        clock::time_point m_start_time;
};


#endif
