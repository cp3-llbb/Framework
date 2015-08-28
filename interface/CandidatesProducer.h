#ifndef CANDIDATES_PRODUCER
#define CANDIDATES_PRODUCER

#include <CommonTools/Utils/interface/StringCutObjectSelector.h>

#include <cp3_llbb/Framework/interface/Producer.h>

template<typename ObjectType>
class CandidatesProducer: public Framework::Producer {
    public:
        CandidatesProducer(const std::string& name, ROOT::TreeGroup& tree, const edm::ParameterSet& config):
            Producer(name, tree, config),
            m_cut(config.getUntrackedParameter<std::string>("cut", "1"))
        {
            // Empty
        }

        virtual ~CandidatesProducer() {}

    protected:
        template<typename T, typename U>
            void fill_candidate(const T& p, const U* gen) {
                p4.push_back(LorentzVector(p.pt(), p.eta(), p.phi(), p.energy()));
                y.push_back(p.rapidity());
                charge.push_back(p.charge());
                if( gen != 0 )
                {
                    matched.push_back(true);
                    gen_p4.push_back(LorentzVector(gen->pt(), gen->eta(), gen->phi(), gen->energy()));
                    gen_y.push_back(gen->y());
                    gen_charge.push_back(gen->charge());
                } else {
                    matched.push_back(false);
                    gen_p4.push_back(LorentzVector(0., 0., 0., 0.));
                    gen_y.push_back(0.);
                    gen_charge.push_back(0.);
                }
            }

        bool pass_cut(const ObjectType& p) {
            return m_cut(p);
        }

    private:
        // Cut
        StringCutObjectSelector<ObjectType> m_cut;

    public:
        // Tree members
        std::vector<LorentzVector>& p4 = tree["p4"].write<std::vector<LorentzVector>>();
        std::vector<float>& y = tree["y"].write<std::vector<float>>();
        std::vector<int8_t>& charge = tree["charge"].write<std::vector<int8_t>>();

        std::vector<bool>& matched = tree["has_matched_gen_particle"].write<std::vector<bool>>();
        std::vector<LorentzVector>& gen_p4 = tree["gen_p4"].write<std::vector<LorentzVector>>();
        std::vector<float>& gen_y = tree["gen_y"].write<std::vector<float>>();
        std::vector<int8_t>& gen_charge = tree["gen_charge"].write<std::vector<int8_t>>();

};

#endif


