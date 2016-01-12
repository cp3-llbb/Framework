#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>
#include <map>
#include <vector>

#include <cp3_llbb/Framework/interface/Cut.h>
#include <cp3_llbb/Framework/interface/ProducersManager.h>
#include <cp3_llbb/Framework/interface/AnalyzersManager.h>

#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>

class CategoryManager;

struct CategoryMetadata {
    // Empty
};

class Category {
    public:
        virtual void configure(const edm::ParameterSet& config) {};

        virtual bool event_in_category_pre_analyzers(const ProducersManager& producers) const = 0;
        virtual bool event_in_category_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) const = 0;

        virtual void register_cuts(CutManager& manager) {};

        virtual void evaluate_cuts_pre_analyzers(CutManager& manager, const ProducersManager& producers) const {};
        virtual void evaluate_cuts_post_analyzers(CutManager& manager, const ProducersManager& producers, const AnalyzersManager& analyzers) const {};

        virtual std::shared_ptr<CategoryMetadata> get_metadata() final {
            return metadata;
        };

    protected:
        std::shared_ptr<CategoryMetadata> metadata;
};

struct CategoryData {
    std::string name;
    std::string description;
    std::unique_ptr<Category> callback;
    ROOT::TreeGroup tree;
    CutManager cut_manager;

    uint64_t events = 0;

    // Tree branches
    bool in_category_pre = false;
    bool in_category_post = false;
    bool& in_category;

    CategoryData(const std::string& name_, const std::string& description_, std::unique_ptr<Category> category, ROOT::TreeWrapper& tree_):
        name(name_),
        description(description_),
        callback(std::move(category)),
        tree(tree_.group(name_ + "_")),
        cut_manager(*this),
        in_category(tree["category"].write<bool>())
    {
        callback->register_cuts(cut_manager);
    }
};

struct CategoryWrapper {
    public:
        CategoryWrapper(const CategoryData& d): data(d) {
            // Empty
        }

        bool in_category() const {
            return data.in_category_pre;
        }

        bool cut_passed(const std::string& cut_name) const {
            return data.cut_manager.cut_passed(cut_name);
        }

        std::shared_ptr<CategoryMetadata> get_metadata() const {
            return data.callback->get_metadata();
        }

    private:
        const CategoryData& data;
};

class CategoryManager {
    friend class ExTreeMaker;

    public:
        template<class T>
        void new_category(const std::string& name, const std::string& description, const edm::ParameterSet& config) {
            std::string internal_name = m_current_prefix + name;
            if (m_categories.count(internal_name) > 0) {
                throw edm::Exception(edm::errors::InsertFailure, "A category named '" + name + "' already exists for this analyzer.");
            }

            std::unique_ptr<Category> category(new T());
            category->configure(config);
            m_categories.emplace(internal_name, CategoryData(internal_name, description, std::move(category), m_tree));
        }

        bool in_category(const std::string& name) const {

            std::string internal_name = m_current_prefix + name;
            const auto& category = m_categories.find(internal_name);
            if (category == m_categories.end()) {
                std::stringstream details;
                details << "Category '" << name << "' not found.";
                throw edm::Exception(edm::errors::NotFound, details.str());
            }

            return category->second.in_category_pre;
        }

        const CategoryWrapper get(const std::string& name) const {

            std::string internal_name = m_current_prefix + name;
            const auto& category = m_categories.find(internal_name);
            if (category == m_categories.end()) {
                std::stringstream details;
                details << "Category '" << name << "' not found.";
                throw edm::Exception(edm::errors::NotFound, details.str());
            }

            return CategoryWrapper(category->second);
        }

    private:
        CategoryManager(ROOT::TreeWrapper& tree):
            m_tree(tree)
        {
            // Empty
        }

        void set_prefix(const std::string& prefix);

        bool evaluate_pre_analyzers(const ProducersManager& producers);
        bool evaluate_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers);

        void reset();

        void print_summary();

        std::unordered_map<std::string, CategoryData> m_categories;
        ROOT::TreeWrapper& m_tree;

        uint64_t processed_events = 0;
        uint64_t selected_events = 0;

        std::string m_current_prefix;
};

#endif
