#include <cp3_llbb/Framework/interface/Category.h>

#include <cstdio>
#include <cinttypes>

bool CategoryManager::evaluate_pre_analyzers(const ProducersManager& producers) {
    bool ret = m_categories.empty();

    for (auto& category: m_categories) {
        if (category.second.callback->event_in_category_pre_analyzers(producers)) {
            ret = true;
            category.second.in_category_pre = true;
            category.second.callback->evaluate_cuts_pre_analyzers(category.second.cut_manager, producers);
        }
    }

    return ret;
}

bool CategoryManager::evaluate_post_analyzers(const ProducersManager& producers, const AnalyzersManager& analyzers) {
    bool ret = m_categories.empty();

    for (auto& category: m_categories) {
        if (category.second.in_category_pre && category.second.callback->event_in_category_post_analyzers(producers, analyzers)) {
            ret = true;
            category.second.in_category_post = true;
            category.second.in_category = true;
            category.second.events++;
            category.second.callback->evaluate_cuts_post_analyzers(category.second.cut_manager, producers, analyzers);
        }
    }

    processed_events++;
    if (ret)
        selected_events++;

    return ret;
}

void CategoryManager::reset() {
    for (auto& category: m_categories) {
        category.second.in_category_pre = false;
        category.second.in_category_post = false;
    }
}

void CategoryManager::print_summary() {

    printf("%" PRIu64 " processed events, %" PRIu64 " selected.\n", processed_events, selected_events);

    if (m_categories.size() == 0)
        return;

    printf("%-60s %20s\n", "Category", "# events");
    printf("---------------------------------------------------------------------------------\n");
    for (auto& category: m_categories)
        printf("%-60s %20" PRIu64 "\n", category.second.name.c_str(), category.second.events);
}

void CategoryManager::set_prefix(const std::string& prefix) {
    m_current_prefix = prefix;
}
