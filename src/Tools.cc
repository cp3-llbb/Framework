#include <cp3_llbb/Framework/interface/Tools.h>
#include <DataFormats/PatCandidates/interface/Jet.h>

#include <fstream>

namespace Tools {
    namespace Jets {

        // Jet id: https://twiki.cern.ch/twiki/bin/view/CMS/JetID#Recommendations_for_13_TeV_data
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
        bool passLooseId(const pat::Jet& jet) {
            DECLARE_QUANTITIES(jet);

            if (fabs(eta) <= 2.7) {
                return (NHF < 0.99 && NEMF < 0.99 && NumConst > 1) && ((fabs(eta) <= 2.4 && CHF > 0 && CHM > 0 && CEMF < 0.99) || fabs(eta) > 2.4) && fabs(eta) <= 2.7;
            } else if (fabs(eta) > 2.7 && fabs(eta) <= 3.0) {
                return (NEMF < 0.90 && NumNeutralParticles > 2 && fabs(eta) > 2.7 && fabs(eta) <= 3.0);
            } else {
                return (NEMF < 0.90 && NumNeutralParticles > 10 && fabs(eta)>3.0);
            }
        }

        bool passTightId(const pat::Jet& jet) {
            DECLARE_QUANTITIES(jet);

            if (fabs(eta) <= 2.7) {
                return (NHF < 0.90 && NEMF < 0.90 && NumConst > 1) && ((fabs(eta) <= 2.4 && CHF > 0 && CHM > 0 && CEMF < 0.99) || fabs(eta) > 2.4) && fabs(eta) <= 2.7;
            } else if (fabs(eta) > 2.7 && fabs(eta) <= 3.0) {
                return (NEMF < 0.90 && NumNeutralParticles > 2 && fabs(eta) > 2.7 && fabs(eta)<=3.0);
            } else {
                return (NEMF < 0.90 && NumNeutralParticles > 10 && fabs(eta) > 3.0);
            }
        }

        bool passTightLeptonVetoId(const pat::Jet& jet) {
            DECLARE_QUANTITIES(jet);

            return (NHF < 0.90 && NEMF < 0.90 && NumConst > 1 && MUF < 0.8) && ((fabs(eta) <= 2.4 && CHF > 0 && CHM > 0 && CEMF < 0.90) || fabs(eta) > 2.4) && fabs(eta) <= 2.7;
        }
#pragma GCC diagnostic pop
    };

    double process_mem_usage() {
        using std::ios_base;
        using std::ifstream;
        using std::string;

        double resident_set = 0.0;

        // 'file' stat seems to give the most reliable results
        //
        ifstream stat_stream("/proc/self/stat",ios_base::in);

        // dummy vars for leading entries in stat that we don't care about
        //
        string pid, comm, state, ppid, pgrp, session, tty_nr;
        string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        string utime, stime, cutime, cstime, priority, nice;
        string O, itrealvalue, starttime;

        // the two fields we want
        //
        unsigned long vsize;
        long rss;

        stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
            >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
            >> utime >> stime >> cutime >> cstime >> priority >> nice
            >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

        stat_stream.close();

        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
        resident_set = rss * page_size_kb;

        return resident_set / 1024;
    }
};
