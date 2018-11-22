#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <Math/Vector4D.h>

namespace {
    struct dictionary {
        std::vector<int8_t> dummy;
        std::vector<int16_t> dummy2;
        std::vector<uint16_t> dummy3;
        std::vector<std::vector<uint16_t>> dummy4;
        std::vector<std::map<std::string, bool>> dummy5;
        std::pair<std::string, bool> dummy6;
        std::vector<std::vector<ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiE4D<float>>>> dummy7;
        std::vector<std::vector<std::string>> dummy8;
	std::vector<std::unordered_map<std::string,float>> dummy9;
    };
}
