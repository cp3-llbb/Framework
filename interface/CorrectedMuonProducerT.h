#pragma once

/** \class CorrectedMuonProducerT
 *
 * Produce collection of corrected muons using either KaMuCa (disabled) or Rochester corrections.
 *
 * \author Sébastien Brochet
 *
 */

#include "CommonTools/Utils/interface/PtComparator.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include <cp3_llbb/Framework/interface/Rochester.h>
// NOTE uncomment also the commented lines in KaMuCaCorrector to re-enable
// #include <KaMuCa/Calibration/interface/KalmanMuonCalibrator.h>
class KalmanMuonCalibrator;

#include <memory>
#include <random>
#include <boost/filesystem.hpp>

namespace cp3 {

    class KaMuCaCorrector {
        public:
            explicit KaMuCaCorrector(const edm::ParameterSet& cfg) {
                std::string tag = cfg.getParameter<std::string>("tag");
                // corrector.reset(new KalmanMuonCalibrator(tag));
            }

            template<typename T>
            T correct(const edm::Event& event, const T& muon) {
                auto corrected_pt = muon.pt(); // corrector->getCorrectedPt(muon.pt(), muon.eta(), muon.phi(), muon.charge());
                // if (! event.isRealData())
                //     corrected_pt = corrector->smear(corrected_pt, muon.eta());

                double ratio = corrected_pt / muon.pt();

                T corrected_muon = muon;
                corrected_muon.setP4(corrected_muon.p4() * ratio);

                return corrected_muon;
            }

        private:
            // std::unique_ptr<KalmanMuonCalibrator> corrector;
    };

    class RochesterCorrector {
        public:
            explicit RochesterCorrector(const edm::ParameterSet& cfg):
                random_generator(42), random_distribution(0, 1) {
                auto tag = cfg.getParameter<edm::FileInPath>("input");

                // Constructor expect a path to the directory, but edm::FileInPath does not support folders
                // Extract the path from the tag file

                boost::filesystem::path p(tag.fullPath());
                corrector.reset(new RoccoR(p.parent_path().native()));
            }

            template<typename T>
            T correct(const edm::Event& event, const T& muon) {

                float scale_factor = 1.;

                if (event.isRealData()) {
                    scale_factor = corrector->kScaleDT(muon.charge(), muon.pt(), muon.eta(), muon.phi(), 0 /* set */, 0 /* param */);
                } else {
                    int n_tracks = 0;
                    if (!muon.innerTrack().isNull())
                        n_tracks = muon.innerTrack()->hitPattern().trackerLayersWithMeasurement();

                    auto gen_particle = muon.genParticle();
                    if (gen_particle)
                        scale_factor = corrector->kScaleFromGenMC(muon.charge(), muon.pt(), muon.eta(), muon.phi(), n_tracks, gen_particle->pt(), random_distribution(random_generator), 0 /* set */, 0 /* param */);
                    else
                        scale_factor = corrector->kScaleAndSmearMC(muon.charge(), muon.pt(), muon.eta(), muon.phi(), n_tracks, random_distribution(random_generator), random_distribution(random_generator), 0 /* set */, 0 /* param */);
                }

                if (std::isnan(scale_factor)) {
                    scale_factor = 1.;
                }

                T corrected_muon = muon;
                corrected_muon.setP4(muon.p4() * scale_factor);

                return corrected_muon;
            }

        private:
            std::unique_ptr<RoccoR> corrector;
            std::mt19937 random_generator;
            std::uniform_real_distribution<double> random_distribution;
    };
}

template <typename T, class C>
class CorrectedMuonProducerT : public edm::stream::EDProducer<> {

    using MuonCollection = std::vector<T>;

    public:
    explicit CorrectedMuonProducerT(const edm::ParameterSet& cfg) {

        enabled = cfg.getParameter<bool>("enabled");
        muons_token = consumes<MuonCollection>(cfg.getParameter<edm::InputTag>("src"));

        if (enabled) {
            corrector.reset(new C(cfg));
        }

        produces<MuonCollection>();
        produces<edm::ValueMap<float>>("correctionFactors");
    }

    virtual void produce(edm::Event& event, const edm::EventSetup& setup) override {

        edm::Handle<MuonCollection> muons_collection;
        event.getByToken(muons_token, muons_collection);

        const MuonCollection& muons = *muons_collection;

        std::unique_ptr<MuonCollection> corrected_muons(new MuonCollection());
        std::vector<float> correction_factors;

        for (const auto& muon: muons) {
            if ((! enabled) || muon.pt() == 0) {
                corrected_muons->push_back(muon);
                correction_factors.push_back(1);
                continue;
            }

            // Correct muon
            T corrected_muon = corrector->template correct<T>(event, muon);

            double ratio = corrected_muon.pt() / muon.pt();

            corrected_muons->push_back(corrected_muon);
            correction_factors.push_back(ratio);
        }

        std::unique_ptr<edm::ValueMap<float>> correction_factors_map(new edm::ValueMap<float>());
        edm::ValueMap<float>::Filler filler(*correction_factors_map);
        filler.insert(muons_collection, correction_factors.begin(), correction_factors.end());
        filler.fill();

        std::sort(corrected_muons->begin(), corrected_muons->end(), muon_pt_comparator);

        event.put(std::move(corrected_muons));
        event.put(std::move(correction_factors_map), "correctionFactors");
    }

    private:
    edm::EDGetTokenT<MuonCollection> muons_token;
    bool enabled;

    std::unique_ptr<C> corrector;

    GreaterByPt<T> muon_pt_comparator;
};
