#pragma once

#include <Math/Vector4D.h>
#include <unordered_map>

#define BRANCH(NAME, ...) __VA_ARGS__& NAME = tree[#NAME].write<__VA_ARGS__>()
#define TRANSIENT_BRANCH(NAME, ...) __VA_ARGS__& NAME = tree[#NAME].transient_write<__VA_ARGS__>()
#define ONLY_NOMINAL_BRANCH(NAME, ...) __VA_ARGS__& NAME = Framework::condition_branch<__VA_ARGS__>(tree, #NAME, !doingSystematics())

typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiE4D<float>> LorentzVector;
typedef std::unordered_map<std::string, float> tauDiscriminatorMap;

#include <cp3_llbb/TreeWrapper/interface/TreeWrapper.h>
namespace Framework {
    template <typename T>
    T& condition_branch(ROOT::TreeGroup& tree, const std::string& branch_name, bool condition) {
        if (condition) {
            return tree[branch_name].write<T>();
        } else {
            return tree[branch_name].transient_write<T>();
        }
    }

  template<typename Type>
    std::set<Type> union_sets(std::initializer_list<std::set<Type>> sets)
    {
      std::set<Type> result;
      for(const auto& set : sets)
	result.insert(set.begin(), set.end());
      return result;
    }


}
