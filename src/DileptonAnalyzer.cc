#include <cp3_llbb/Framework/interface/DileptonAnalyzer.h>
#include <cp3_llbb/Framework/interface/DileptonCategories.h>

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/ElectronsProducer.h>
#include <cp3_llbb/Framework/interface/MuonsProducer.h>

template <>
bool DileptonAnalyzer::passID(const MuonsProducer& muons, size_t index) {
    switch (m_muons_wp) {
        case WorkingPoint::LOOSE:
            return muons.isLoose[index];

        case WorkingPoint::TIGHT:
            return muons.isTight[index];
    }

    return false;
}

template <>
bool DileptonAnalyzer::passID(const ElectronsProducer& electrons, size_t index) {
    switch (m_electrons_wp) {
        case WorkingPoint::LOOSE:
            return electrons.ids[index][m_electron_loose_wp_name];

        case WorkingPoint::TIGHT:
            return electrons.ids[index][m_electron_tight_wp_name];
    }

    return false;
}


void DileptonAnalyzer::registerCategories(CategoryManager& manager, const edm::ParameterSet& config) {
    if (m_standalone_mode) {
        manager.new_category<MuMuCategory>("mumu", "Category with leading leptons as two muons", config);
        manager.new_category<ElElCategory>("elel", "Category with leading leptons as two electrons", config);
        manager.new_category<MuElCategory>("muel", "Category with leading leptons as muon, electron", config);
        manager.new_category<ElMuCategory>("elmu", "Category with leading leptons as electron, muon", config);
    }
}

void DileptonAnalyzer::analyze(const edm::Event& event, const edm::EventSetup&, const ProducersManager& producers, const AnalyzersManager& analyzers, const CategoryManager& categories) {

// ***** ***** *****
// Get all dilepton objects out of the event
// ***** ***** *****
    const MuonsProducer& muons = producers.get<MuonsProducer>("muons");
    const ElectronsProducer& electrons = producers.get<ElectronsProducer>("electrons");

    // Dimuons
    for( unsigned int imuon = 0 ; imuon < muons.p4.size() ; imuon++ )
    {
        if(passID(muons, imuon))
        {
            for( unsigned int jmuon = imuon+1 ; jmuon < muons.p4.size() ; jmuon++ )
            {
                if(passID(muons, jmuon) && muons.charge[imuon] * muons.charge[jmuon] < 0 )
                {
                    LorentzVector dimuon = muons.p4[imuon] + muons.p4[jmuon];
                    mumu.push_back(dimuon);
                    mumu_indices.push_back(std::make_pair(imuon, jmuon));
                }
            }
        }
    }

    // Dielectrons
    for( unsigned int ielectron = 0 ; ielectron < electrons.p4.size() ; ielectron++ )
    {
        if(passID(electrons, ielectron))
        {
            for( unsigned int jelectron = ielectron+1 ; jelectron < electrons.p4.size() ; jelectron++ )
            {
                if(passID(electrons, jelectron) && electrons.charge[ielectron] * electrons.charge[jelectron] < 0 )
                {
                    LorentzVector dielectron = electrons.p4[ielectron] + electrons.p4[jelectron];
                    elel.push_back(dielectron);
                    elel_indices.push_back(std::make_pair(ielectron, jelectron));
                }
            }
        }
    }

    //Dilepton electron-muon
    for( unsigned int ielectron = 0 ; ielectron < electrons.p4.size() ; ielectron++ )
    {
        if(passID(electrons, ielectron))
        {
            for( unsigned int jmuon = 0 ; jmuon < muons.p4.size() ; jmuon++ )
            {
                if( muons.p4[jmuon].pt() < electrons.p4[ielectron].pt() &&
                    passID(muons, jmuon) &&
                    electrons.charge[ielectron] * muons.charge[jmuon] < 0 )
                {
                    LorentzVector dilepton = electrons.p4[ielectron] + muons.p4[jmuon];
                    elmu.push_back(dilepton);
                    elmu_indices.push_back(std::make_pair(ielectron, jmuon));
                }
            }
        }
    }

    //Dilepton muon-electron
    for( unsigned int imuon = 0 ; imuon < muons.p4.size() ; imuon++ )
    {
        if(passID(muons, imuon))
        {
            for( unsigned int jelectron = 0 ; jelectron < electrons.p4.size() ; jelectron++ )
            {
                if( electrons.p4[jelectron].pt() < muons.p4[imuon].pt() &&
                    passID(electrons, jelectron) &&
                    muons.charge[imuon] * electrons.charge[jelectron] < 0 )
                {
                    LorentzVector dilepton = muons.p4[imuon] + electrons.p4[jelectron];
                    muel.push_back(dilepton);
                    muel_indices.push_back(std::make_pair(imuon, jelectron));
                }
            }
        }
    }

}

