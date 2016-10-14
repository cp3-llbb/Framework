
#include <cp3_llbb/Framework/interface/HLTProducer.h>

void HLTProducer::produce(edm::Event& event, const edm::EventSetup& eventSetup) {

    edm::Handle<edm::TriggerResults> hlt;
    event.getByToken(m_hlt_token, hlt);

    edm::Handle<pat::PackedTriggerPrescales> prescales_;
    event.getByToken(m_prescales_token, prescales_);

    const edm::TriggerNames& triggerNames = event.triggerNames(*hlt);

    bool filter = m_hlt_service.get() != nullptr;
    const HLTService::PathVector* valid_paths = nullptr;
    if (filter) {
        valid_paths = &m_hlt_service->getPaths(event.id().run());
    }

    for (size_t i = 0 ; i < hlt->size(); i++) {
        if (hlt->accept(i)) {
            std::string triggerName = triggerNames.triggerName(i);
            if (triggerName == "HLTriggerFinalPath")
                continue; // This one is pretty useless...
            if (triggerName[0] == 'A')
                continue; // Remove AlCa HLT paths

            bool add = false;
            if (filter) {
                for (const auto& regex: *valid_paths) {
                    if (boost::regex_match(triggerName, regex)) {
                        add = true;
                        break;
                    }
                }
            } else {
                add = true;
            }

            if (add) {
                paths.push_back(triggerName);
                if (prescales_.isValid()) {
                    prescales.push_back(prescales_->getPrescaleForIndex(i));
                }
            }
        }
    }

    if (paths.empty())
        return;

    std::sort(paths.begin(), paths.end());

    edm::Handle<pat::TriggerObjectStandAloneCollection> objects;
    event.getByToken(m_trigger_objects_token, objects);

    if (objects.isValid()) {
    
        for (pat::TriggerObjectStandAlone obj : *objects) {
            obj.unpackPathNames(triggerNames);

            std::vector<std::string> object_paths_ = obj.pathNames(false);

            // Check if this object has triggered at least one of the path we are interesting in
            bool keep = false;
            if (filter) {
                for (const auto& path: object_paths_) {
                    if (std::find(paths.begin(), paths.end(), path) != paths.end()) {
                        keep = true;
                        break;
                    }
                }
            } else {
                keep = true;
            }

            if (! keep)
                continue;

            std::sort(object_paths_.begin(), object_paths_.end());

            std::vector<std::string> filtered_paths;
            std::set_intersection(paths.begin(), paths.end(), object_paths_.begin(), object_paths_.end(), std::back_inserter(filtered_paths));

            object_paths.push_back(filtered_paths);
            object_filters.push_back(obj.filterLabels());
            object_p4.push_back(LorentzVector(obj.pt(), obj.eta(), obj.phi(), obj.energy()));
            object_pdg_id.push_back(obj.pdgId());
        }
    }
}
