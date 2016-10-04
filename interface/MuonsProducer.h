#ifndef MUONS_PRODUCER
#define MUONS_PRODUCER

#include <cp3_llbb/Framework/interface/LeptonsProducer.h>
#include <cp3_llbb/Framework/interface/ScaleFactors.h>

#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/PatCandidates/interface/Muon.h>

#include <cp3_llbb/Framework/interface/rochcor2016.h>
#include <KaMuCa/Calibration/interface/KalmanMuonCalibrator.h>

#include <utility>

class MuonsProducer: public LeptonsProducer<pat::Muon>, public ScaleFactors {
    public:
        MuonsProducer(const std::string& name, const ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            LeptonsProducer(name, tree, config), ScaleFactors(const_cast<ROOT::TreeGroup&>(tree))
        {
            ScaleFactors::create_branches(config);
            if (config.exists("applyRochester")) {
                applyRochester = config.getUntrackedParameter<bool>("applyRochester");
                rochesterInputFile = config.getUntrackedParameter<edm::FileInPath>("rochesterInputFile");
            }
            if (config.exists("applyKaMuCa")) {
                applyKaMuCa = config.getUntrackedParameter<bool>("applyKaMuCa");
                // The inputTagKaMuCa is decided to be either of inputTagKaMuCaData or inputTagKaMuCaMC (read from python/MuonsProducer.py) in Framework.py
                inputTagKaMuCa = config.getUntrackedParameter<std::string>("inputTagKaMuCa");
            }
            if (applyRochester || applyKaMuCa) {
                if (applyRochester && applyKaMuCa) {
                    throw edm::Exception(edm::errors::Configuration, "Trying to apply two different muon momentum corrections");
                } else if (applyRochester) {
                    std::cout << "  -> applying rochester muon momentum corrections, with input file " << rochesterInputFile.fullPath() << std::endl;
                    rmcor.reset(new rochcor2016(rochesterInputFile.fullPath()));
                } else if (applyKaMuCa) {
                    std::cout << "  -> applying Kalman muon calibrator (KaMuCa), with input tag: " << inputTagKaMuCa << std::endl;
                    kamucacor.reset(new KalmanMuonCalibrator(inputTagKaMuCa));
                }
            }
        }

        virtual ~MuonsProducer() {}

        virtual void doConsumes(const edm::ParameterSet& config, edm::ConsumesCollector&& collector) override {
            LeptonsProducer::doConsumes(config, std::forward<edm::ConsumesCollector>(collector));

            m_vertices_token = collector.consumes<std::vector<reco::Vertex>>(config.getUntrackedParameter<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices")));
        }

        virtual void produce(edm::Event& event, const edm::EventSetup& eventSetup) override;

    private:
        // Tokens
        edm::EDGetTokenT<std::vector<reco::Vertex>> m_vertices_token;
        bool applyRochester = false ;
        edm::FileInPath rochesterInputFile;
        bool applyKaMuCa = false ;
        std::string inputTagKaMuCa = "";
    public:
        // Tree members
        std::vector<bool>& isLoose = tree["isLoose"].write<std::vector<bool>>();
        std::vector<bool>& isSoft = tree["isSoft"].write<std::vector<bool>>();
        std::vector<bool>& isMedium = tree["isMedium"].write<std::vector<bool>>();
        std::vector<bool>& isTight = tree["isTight"].write<std::vector<bool>>();
        std::vector<bool>& isHighPt = tree["isHighPt"].write<std::vector<bool>>();

        BRANCH(dxy, std::vector<float>);
        BRANCH(dz, std::vector<float>);
        BRANCH(dca, std::vector<float>);
        std::unique_ptr<rochcor2016> rmcor;
        std::unique_ptr<KalmanMuonCalibrator> kamucacor;
        BRANCH(rochester_correction, std::vector<float>);
        BRANCH(kamuca_correction, std::vector<float>);
};

#endif
