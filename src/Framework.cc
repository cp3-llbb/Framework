// -*- C++ -*-
//
// Package:    cp3_llbb/Framework
// Class:      ExTreeMaker
// 
//
// Original Author:  Sébastien Brochet
//         Created:  Fri, 26 Jun 2015 13:40:32 GMT
//
//


// system include files
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <chrono>

// user include files
#include <cp3_llbb/Framework/interface/Framework.h>
#include <cp3_llbb/Framework/interface/Tools.h>
#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

#include <Compression.h>
#include <TFile.h>
#include <TTree.h>

// Uncomment to enable printout about memory usage
//#define DEBUG_MEMORY_USAGE

// Uncomment to debug TTree::Fill
//#define DEBUG_TREE_FILL

// Uncomment to override CMSSW root error handling. Usefull to see all infos from ROOT
//#define OVERRIDE_ROOT_ERROR_HANDLER

#ifdef OVERRIDE_ROOT_ERROR_HANDLER
#include <TError.h>
void RootErrorHandler(int level, bool, char const* location, char const* message) {
    std::cout << "[" << location << "] [ " << level << "] " << message << std::endl;
}
#endif

// Tools for sorting
template <typename T>
std::vector<size_t> get_permutations(const std::vector<T>& from, const std::vector<T>& to) {

    std::vector<size_t> p(from.size());

    std::transform(from.begin(), from.end(), p.begin(), [&from, &to](const T& item) -> size_t {
                return std::distance(to.begin(), std::find(to.begin(), to.end(), item));
            });

    return p;
}

template <typename T>
void apply_permutations(std::vector<T>& vec, std::vector<size_t> const& p) {
    std::vector<T> sorted_vec(p.size());
    std::transform(p.begin(), p.end(), sorted_vec.begin(), [&vec](int i) {
            return vec[i];
            });

    vec = sorted_vec;
}

ExTreeMaker::ExTreeMaker(const edm::ParameterSet& iConfig):
    m_output_filename(iConfig.getParameter<std::string>("output")) {

#ifdef DEBUG_MEMORY_USAGE
        std::cout << "[Framework - >>constructor] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

        // Use default values from CMSSW to optimize TTree output
        m_output.reset(TFile::Open(m_output_filename.c_str(), "recreate", "",
              iConfig.getUntrackedParameter<int32_t>("compressionSettings", 1))); // see ROOT::CompressionSettings
        m_output->cd();

        m_raw_tree = new TTree("t", "t");
        m_flush_size = iConfig.getUntrackedParameter<unsigned long long>("treeFlushSize", 15 * 1024 * 1024);
        m_raw_tree->SetAutoFlush(0);

        // From CMSSW:
        // "Turn off autosaving because it is such a memory hog and we are not using
        // this check-pointing feature anyway."
        m_raw_tree->SetAutoSave(std::numeric_limits<Long64_t>::max());
        m_raw_tree->SetMaxVirtualSize(iConfig.getUntrackedParameter<unsigned long long>("treeMaxVirtualSize", 150 * 1024 * 1024));
        m_wrapper.reset(new ROOT::TreeWrapper(m_raw_tree));

        m_categories.reset(new CategoryManager(*m_wrapper));
        m_producers_manager.reset(new ProducersManager(*this));
        m_analyzers_manager.reset(new AnalyzersManager(*this));

        m_metadata.reset(new MetadataManager(m_output.get()));

        // Load plugins
        if (!iConfig.existsAs<edm::ParameterSet>("producers")) {
            throw new std::logic_error("No producers specified");
        }

        std::cout << std::endl << "filters: " << std::endl;
        const edm::ParameterSet& filters = iConfig.getParameterSet("filters");
        std::vector<std::string> filtersName = filters.getParameterNames();
        for (std::string& filterName: filtersName) {
            edm::ParameterSet filterData = filters.getParameterSet(filterName);
            bool enable = filterData.getParameter<bool>("enable");
            if (! enable)
                continue;

            const std::string type = filterData.getParameter<std::string>("type");
            edm::ParameterSet filterParameters;
            if (filterData.existsAs<edm::ParameterSet>("parameters"))
                filterParameters = filterData.getParameterSet("parameters");

            std::cout << " -> Adding filter '" << filterName << "' of type '" << type << "'" << std::endl;
            auto filter = std::shared_ptr<Framework::Filter>(ExTreeMakerFilterFactory::get()->create(type, filterName, filterParameters));
            filter->doConsumes(filterParameters, consumesCollector());

            m_filters.emplace(filterName, filter);
        }

        std::cout << std::endl << "producers: " << std::endl;
        const edm::ParameterSet& producers = iConfig.getParameterSet("producers");
        std::vector<std::string> producersName = producers.getParameterNames();

        if (iConfig.exists("producers_scheduling")) {
            const std::vector<std::string>& scheduling = iConfig.getUntrackedParameter<std::vector<std::string>>("producers_scheduling");
            auto p = get_permutations(scheduling, producersName);

            apply_permutations(producersName, p);
        }

        for (std::string& producerName: producersName) {
            edm::ParameterSet producerData = producers.getParameterSet(producerName);
            bool enable = producerData.getParameter<bool>("enable");
            if (! enable)
                continue;

            const std::string type = producerData.getParameter<std::string>("type");
            const std::string tree_prefix = producerData.getParameter<std::string>("prefix");
            edm::ParameterSet producerParameters;
            if (producerData.existsAs<edm::ParameterSet>("parameters"))
                producerParameters = producerData.getParameterSet("parameters");

            std::cout << " -> Adding producer '" << producerName << "' of type '" << type << "'" << std::endl;
            auto producer = std::shared_ptr<Framework::Producer>(ExTreeMakerProducerFactory::get()->create(type, producerName, m_wrapper->group(tree_prefix), producerParameters));
            producer->doConsumes(producerParameters, consumesCollector());

            m_producers.push_back(std::make_pair(producerName, producer));
        }

        if (!iConfig.existsAs<edm::ParameterSet>("analyzers")) {
            return;
        }

        std::cout << std::endl << "analyzers: " << std::endl;
        const edm::ParameterSet& analyzers = iConfig.getParameterSet("analyzers");
        std::vector<std::string> analyzersName = analyzers.getParameterNames();

        if (iConfig.exists("analyzers_scheduling")) {
            const std::vector<std::string>& scheduling = iConfig.getUntrackedParameter<std::vector<std::string>>("analyzers_scheduling");
            auto p = get_permutations(scheduling, analyzersName);

            apply_permutations(analyzersName, p);
        }

        for (std::string& analyzerName: analyzersName) {
            edm::ParameterSet analyzerData = analyzers.getParameterSet(analyzerName);
            bool enable = analyzerData.getParameter<bool>("enable");
            if (! enable)
                continue;

            const std::string type = analyzerData.getParameter<std::string>("type");
            const std::string tree_prefix = analyzerData.getParameter<std::string>("prefix");
            edm::ParameterSet analyzerParameters;
            if (analyzerData.exists("parameters"))
                analyzerParameters = analyzerData.getParameterSet("parameters");

            auto analyzer = std::shared_ptr<Framework::Analyzer>(ExTreeMakerAnalyzerFactory::get()->create(type, analyzerName, m_wrapper->group(tree_prefix), analyzerParameters));
            analyzer->doConsumes(analyzerParameters, consumesCollector());

            edm::ParameterSet analyzerCategoriesParameters;
            if (analyzerData.exists("categories_parameters")) {
                analyzerCategoriesParameters = analyzerData.getParameterSet("categories_parameters");
            }

            m_categories->set_prefix(tree_prefix);
            analyzer->registerCategories(*m_categories, analyzerCategoriesParameters);
            m_categories->set_prefix("");


            std::cout << " -> Adding analyzer '" << analyzerName << "'" << std::endl;

            m_analyzers.push_back({analyzer, analyzerName, tree_prefix});
            m_analyzers_name.push_back(analyzerName);
        }

#ifdef DEBUG_MEMORY_USAGE
        std::cout << "[Framework - <<constructor] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
        std::cout << std::endl;
}

// to silence warning, and because inherits a throwing constructor, so they need to be all consistent
ProducerGetter::~ProducerGetter() noexcept(false) {}
AnalyzerGetter::~AnalyzerGetter() noexcept(false) {}

ExTreeMaker::~ExTreeMaker() {

}


void ExTreeMaker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    bool should_continue = true;

#ifdef DEBUG_MEMORY_USAGE
        ErrorHandlerFunc_t defaultRootErrorHandler = GetErrorHandler();
        SetErrorHandler(RootErrorHandler);
#endif

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>produce] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

    for (auto& filter: m_filters)
        should_continue &= filter.second->filter(iEvent, iSetup);

    if (! should_continue)
        return;

    for (auto& producer: m_producers) {
        producer.second->produce(iEvent, iSetup);
        producer.second->setRun(true);
    }

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>produce after producers] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

    should_continue = m_categories->evaluate_pre_analyzers(*m_producers_manager);
    if (! should_continue) {
        m_wrapper->reset();
        m_categories->reset();
        return;
    }

    for (auto& analyzer: m_analyzers) {
        m_categories->set_prefix(analyzer.prefix);
        analyzer.analyzer->analyze(iEvent, iSetup, *m_producers_manager, *m_analyzers_manager, *m_categories);
        analyzer.analyzer->setRun(true);
    }
    m_categories->set_prefix("");

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>produce after analyzers] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

    if (m_categories->evaluate_post_analyzers(*m_producers_manager, *m_analyzers_manager)) {
#ifdef DEBUG_TREE_FILL
        gDebug = 1;
#endif

        size_t zipSize = m_raw_tree->GetZipBytes();
        m_wrapper->fillBranches();
        m_filled_size += (m_raw_tree->GetZipBytes() - zipSize);

        if (m_filled_size > m_flush_size) {
#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>produce before flushing] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
            m_raw_tree->FlushBaskets();
            m_filled_size = 0;

            if (! m_baskets_optimized) {
                m_baskets_optimized = true;
                m_raw_tree->OptimizeBaskets(m_raw_tree->GetTotBytes(), 1 ,"");
            }

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>produce after flushing] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
        }

#ifdef DEBUG_TREE_FILL
        gDebug = 0;
#endif
    } else
        m_wrapper->reset();

    m_categories->reset();

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->setRun(false);

    for (auto& producer: m_producers)
        producer.second->setRun(false);

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - <<produce] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
#ifdef OVERRIDE_ROOT_ERROR_HANDLER
        SetErrorHandler(defaultRootErrorHandler);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void ExTreeMaker::beginJob() {
    m_start_time = clock::now();

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>beginJob] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

    for (auto& filter: m_filters)
        filter.second->beginJob(*m_metadata);

    for (auto& producer: m_producers)
        producer.second->beginJob(*m_metadata);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->beginJob(*m_metadata);

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - <<beginJob] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
}

// ------------ method called once each job just after ending the event loop  ------------
void ExTreeMaker::endJob() {

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - >>endJob] RSS: " << Tools::process_mem_usage() << std::endl;
#endif

    // This is needed since we don't fill the tree directory, but each branch separately
    m_raw_tree->SetEntries(-1);

    std::cout << std::endl << "---" << std::endl;
    for (auto& filter: m_filters)
        filter.second->endJob(*m_metadata);

    for (auto& producer: m_producers)
        producer.second->endJob(*m_metadata);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->endJob(*m_metadata);

    auto end_time = clock::now();

    std::cout << std::endl << "Job done in " << std::chrono::duration_cast<ms>(end_time - m_start_time).count() / 1000. << "s" << std::endl;

    m_raw_tree->AutoSave("FlushBaskets Overwrite");
    m_output->Write("", TObject::kOverwrite);

    m_categories->print_summary();

#ifdef DEBUG_MEMORY_USAGE
    std::cout << "[Framework - <<endJob] RSS: " << Tools::process_mem_usage() << std::endl;
#endif
}

void ExTreeMaker::beginRun(const edm::Run& run, const edm::EventSetup& eventSetup) {
    for (auto& filter: m_filters)
        filter.second->beginRun(run, eventSetup);

    for (auto& producer: m_producers)
        producer.second->beginRun(run, eventSetup);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->beginRun(run, eventSetup);

}

void ExTreeMaker::endRun(const edm::Run& run, const edm::EventSetup& eventSetup) {
    for (auto& filter: m_filters)
        filter.second->endRun(run, eventSetup);

    for (auto& producer: m_producers)
        producer.second->endRun(run, eventSetup);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->endRun(run, eventSetup);
}

void ExTreeMaker::beginLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& eventSetup) {
    for (auto& filter: m_filters)
        filter.second->beginLuminosityBlock(lumi, eventSetup);

    for (auto& producer: m_producers)
        producer.second->beginLuminosityBlock(lumi, eventSetup);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->beginLuminosityBlock(lumi, eventSetup);

}

void ExTreeMaker::endLuminosityBlock(const edm::LuminosityBlock& lumi, const edm::EventSetup& eventSetup) {
    for (auto& filter: m_filters)
        filter.second->endLuminosityBlock(lumi, eventSetup);

    for (auto& producer: m_producers)
        producer.second->endLuminosityBlock(lumi, eventSetup);

    for (auto& analyzer: m_analyzers)
        analyzer.analyzer->endLuminosityBlock(lumi, eventSetup);
}

const Framework::Producer& ExTreeMaker::getProducer(const std::string& name) const {
    const auto producer = std::find_if(m_producers.begin(), m_producers.end(), [&name](const std::pair<std::string, std::shared_ptr<Framework::Producer>>& element) { return element.first == name; });
    if (producer == m_producers.end()) {
        std::stringstream details;
        details << "Producer '" << name << "' not found. Please load it first in the python configuration";
        throw edm::Exception(edm::errors::NotFound, details.str());
    }

    Framework::Producer& p = *producer->second;
    if (! p.hasRun()) {
        std::stringstream details;
        details << "Producer '" << name << "' has not been run yet for this event. Please check the scheduling of your producers";
        throw edm::Exception(edm::errors::NotFound, details.str());
    }

    return p;
}

bool ExTreeMaker::producerExists(const std::string& name) const {
    const auto producer = std::find_if(m_producers.begin(), m_producers.end(), [&name](const std::pair<std::string, std::shared_ptr<Framework::Producer>>& element) { return element.first == name; });
    return (producer != m_producers.end());
}

const Framework::Analyzer& ExTreeMaker::getAnalyzer(const std::string& name) const {
    auto it = std::find(m_analyzers_name.begin(), m_analyzers_name.end(), name);
    if (it == m_analyzers_name.end()) {
        std::stringstream details;
        details << "Analyzer '" << name << "' not found. Please load it first in the python configuration";
        throw edm::Exception(edm::errors::NotFound, details.str());
    }

    Framework::Analyzer& analyzer = *m_analyzers[std::distance(m_analyzers_name.begin(), it)].analyzer;
    if (! analyzer.hasRun()) {
        std::stringstream details;
        details << "Analyzer '" << name << "' has not been run yet for this event. Please check the scheduling of your analyzers";
        throw edm::Exception(edm::errors::NotFound, details.str());
    }

    return analyzer;
}

bool ExTreeMaker::analyzerExists(const std::string& name) const {
    const auto& analyzer = std::find(m_analyzers_name.begin(), m_analyzers_name.end(), name);
    return (analyzer != m_analyzers_name.end());
}

//define this as a plug-in
DEFINE_FWK_MODULE(ExTreeMaker);
