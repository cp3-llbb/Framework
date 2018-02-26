#pragma once

#include <algorithm>
#include <string>

namespace pat {
    class Jet;
}

namespace Tools {
    namespace Jets {

        // Jet id: https://twiki.cern.ch/twiki/bin/view/CMS/JetID#Recommendations_for_13_TeV_data
#define DECLARE_QUANTITIES(jet) \
        float eta = jet.eta(); \
        float NHF = jet.neutralHadronEnergyFraction(); \
        float NEMF = jet.neutralEmEnergyFraction(); \
        float CHF = jet.chargedHadronEnergyFraction(); \
        float MUF = jet.muonEnergyFraction(); \
        float CEMF = jet.chargedEmEnergyFraction(); \
        size_t NumConst = jet.chargedMultiplicity() + jet.neutralMultiplicity(); \
        size_t NumNeutralParticles = jet.neutralMultiplicity(); \
        size_t CHM = jet.chargedMultiplicity();

        bool passLooseId(const pat::Jet& jet);
        bool passTightId(const pat::Jet& jet);
        bool passTightLeptonVetoId(const pat::Jet& jet);
    };

    inline bool caseInsensitiveEquals(const std::string& a, const std::string& b) {
        return std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    }

    double process_mem_usage();
};
