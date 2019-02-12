#define CATCH_CONFIG_RUNNER
#include <cp3_llbb/Framework/interface/catch.hpp>

#include <memory>

#define private public
#define protected public
#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>
#include <TBasket.h>
#undef private
#undef protected

#include <TBufferFile.h>
#include <TLeaf.h>
#include <TParameter.h>

#include <openssl/sha.h>

struct Config {
    std::string reference_file;
    std::string test_file;
};

Config config;

std::string sha256(char* buffer, int length) {

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buffer, length);
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for(size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
    }

    return ss.str();
}

#include <iostream>
#include <iomanip>
inline void print_bytes(char const * buffer, std::size_t count, std::size_t bytes_per_line, std::ostream & out) {
    std::ios::fmtflags flags(out.flags()); // Save flags before manipulation.
    out << std::hex << std::setfill('0');
    out.setf(std::ios::uppercase);
    for (std::size_t i = 0; i != count; ++i) {
        auto current_byte_number = static_cast<unsigned int>(static_cast<unsigned char>(buffer[i]));
        out << std::setw(2) << current_byte_number;
        bool is_end_of_line = (bytes_per_line != 0) && ((i + 1 == count) || ((i + 1) % bytes_per_line == 0));
        out << (is_end_of_line ? '\n' : ' ');
    }
    out.flush();
    out.flags(flags); // Restore original flags.
}

namespace {
  template<typename T>
  struct areEqualFloat_ {
    bool operator() (const T& a, const T& b, const std::string&) const { return a == b; } // trivial case
  };
  template<> struct areEqualFloat_<float> {
    bool operator() (const float& a, const float& b, const std::string& msg) const
    {
      if ( a != b ) {
        const auto reldiff = .5*std::abs(a-b)/(a+b);
        if ( reldiff > std::numeric_limits<float>::epsilon() ) {
          return false;
        } else { // within precision: print a warning
          std::cout << "Warning: " << msg << " " << a << " vs " << b
            << " rel.diff=" << reldiff << ", epsilon=" << std::numeric_limits<float>::epsilon() << std::endl;
        }
      }
      return true;
    }
  };
  template<typename T> // vector types
  struct areEqualFloat_<std::vector<T>> {
    bool operator() (const std::vector<T>& a, const std::vector<T>& b, const std::string& msg) const
    {
      if ( a.size() != b.size() ) {
        std::cout << "Error: " << msg << " sizes differ: " << a.size() << " vs " << b.size() << std::endl;
        return false;
      }
      const areEqualFloat_<T> elmComp;
      for ( std::size_t i{0}; i != a.size(); ++i ) {
        if ( ! elmComp(a[i], b[i], msg+"["+std::to_string(i)+"]") ) return false;
      }
      return true;
    }
  };

  template<typename T> // helper method from value void-pointers
  bool areEqualFloat(void* aPtr, void* bPtr, const std::string& msg)
  {
    const areEqualFloat_<T> comp;
    return comp(*reinterpret_cast<const T*>(aPtr), *reinterpret_cast<const T*>(bPtr), msg);
  }

  /**
   * Compare all the entries, return true only if they are identical (within precision, for floating point numbers)
   */
  bool compareBranchValuesFloat(TBranch* ref, TBranch* test, size_t entry)
  {
    // std::cout << "Entry " << entry << ": different hash for branch " << ref->GetClassName() << " " << ref->GetName() << ", trying detailed comparison of the leaves" << std::endl;
    bool areEqual = true;
    const auto& refLvs = *(ref->GetListOfLeaves());
    const auto& testLvs = *(test->GetListOfLeaves());
    auto rlIt = refLvs.begin(); auto tlIt = testLvs.begin();
    for ( ; rlIt != refLvs.end() && tlIt != testLvs.end(); ++rlIt, ++tlIt ) {
      const auto rlI = dynamic_cast<const TLeaf*>(*rlIt);
      const auto tlI = dynamic_cast<const TLeaf*>(*tlIt);
      if ( std::string(rlI->GetName()) != std::string(tlI->GetName()) ) {
        std::cout << "Error: Leaves have different names: " << rlI->GetName() << " vs " << tlI->GetName() << std::endl;
        areEqual = false;
        continue;
      }
      if ( std::string(rlI->GetTypeName()) != std::string(tlI->GetTypeName()) ) {
        std::cout << "Error: Leaves have different typenames: " << rlI->GetTypeName() << " vs " << tlI->GetTypeName() << std::endl;
        areEqual = false;
        continue;
      }
      if ( rlI->GetLeafCount() || tlI->GetLeafCount() ) {
        std::cout << "NotImplemented: Variable-length array leaves" << std::endl;
        areEqual = false;
      }
      const std::string msg = std::string("leaf ")+rlI->GetTypeName()+" "+rlI->GetName();
      if ( std::string(rlI->GetTypeName()) == "float" ) {
        const areEqualFloat_<float> comp;
        areEqual = areEqual && comp(rlI->GetValue(), tlI->GetValue(), msg);
      } else if ( std::string(rlI->GetTypeName()) == "vector<float>" ) {
        areEqual = areEqual && areEqualFloat<std::vector<float>>(rlI->GetValuePointer(), tlI->GetValuePointer(), msg);
      } else if ( std::string(rlI->GetTypeName()) == "vector<vector<float> >" ) {
        areEqual = areEqual && areEqualFloat<std::vector<std::vector<float>>>(rlI->GetValuePointer(), tlI->GetValuePointer(), msg);
      } else {
        std::cout << "NotImplemented: type " << rlI->GetTypeName() << std::endl;
        areEqual = false;
      }
    }
    return areEqual;
  }
}

bool diffBranches(size_t entry, TBranch* ref, TBranch* test) {

    if (ref->GetListOfBranches()->GetEntries() > 0) {
        bool identical = true;
	for (size_t n = 0; n < (size_t) ref->GetListOfBranches()->GetEntries(); n++) {
	    TBranch* ref_subbranch = (TBranch*) ref->GetListOfBranches()->UncheckedAt(n);
	    REQUIRE(ref_subbranch);

	    TBranch* test_subbranch = (TBranch*) test->GetListOfBranches()->UncheckedAt(n);
	    REQUIRE(test_subbranch);

            bool i_id = diffBranches(entry, ref_subbranch, test_subbranch); CHECK(i_id);
            identical = identical && i_id;
	}

	return identical;
    }

    INFO("[Event " << entry << "] Checking branches " << ref->GetName() << " of type " << ref->GetClassName());

    ref->GetEntry(entry);
    test->GetEntry(entry);

    bool same_nLeaves = ( test->GetListOfLeaves()->GetEntries() == ref->GetListOfLeaves()->GetEntries() ); CHECK(same_nLeaves);

    TBasket* ref_basket = ref->fCurrentBasket;
    REQUIRE(ref_basket);

    TBasket* test_basket = test->fCurrentBasket;
    REQUIRE(test_basket);

    TBuffer* ref_buffer = ref_basket->GetBufferRef();
    REQUIRE(ref_buffer);

    TBuffer* test_buffer = test_basket->GetBufferRef();
    REQUIRE(test_buffer);

    std::string ref_sha = sha256(ref_buffer->Buffer() + ref_basket->GetKeylen(), ref_buffer->Length() - ref_basket->GetKeylen());
    std::string test_sha = sha256(test_buffer->Buffer() + test_basket->GetKeylen(), test_buffer->Length() - test_basket->GetKeylen());

    bool same_hash = ( ref_sha == test_sha ) || compareBranchValuesFloat(ref, test, entry);

    return ( same_nLeaves && same_hash );
}

TEST_CASE("Check if trees are equals", "[diff]") {
    std::unique_ptr<TFile> ref_file(TFile::Open(config.reference_file.c_str(), "read"));
    std::unique_ptr<TFile> test_file(TFile::Open(config.test_file.c_str(), "read"));

    REQUIRE(ref_file.get());
    REQUIRE(test_file.get());

    TTree* ref_tree = (TTree*) ref_file->Get("framework/t");
    REQUIRE(ref_tree);

    TTree* test_tree = (TTree*) test_file->Get("framework/t");
    REQUIRE(test_tree);

    ref_tree->SetBranchStatus("*", 1);
    test_tree->SetBranchStatus("*", 1);

    size_t n_branches = ref_tree->GetNbranches();
    if ((size_t) test_tree->GetNbranches() != n_branches) {
        std::set<std::string> ref_branches;
        TObjArray* branches = ref_tree->GetListOfBranches();

        TIter next(branches);
        TBranch* branch;
        while ((branch = (TBranch*) next())) {
            ref_branches.emplace(branch->GetName());
        }

        std::set<std::string> test_branches;
        branches = test_tree->GetListOfBranches();
        next = TIter(branches);
        while ((branch = (TBranch*) next())) {
            test_branches.emplace(branch->GetName());
        }

        std::set<std::string> diff;
        std::set_difference(ref_branches.begin(), ref_branches.end(),
                test_branches.begin(), test_branches.end(),
                std::inserter(diff, diff.begin()));

        std::cout << "Branches found in reference tree but not in output tree:" << std::endl;
        for (const auto& b: diff) {
            std::cout << "  " << b << std::endl;
        }

        diff.clear();
        std::set_difference(test_branches.begin(), test_branches.end(),
                ref_branches.begin(), ref_branches.end(),
                std::inserter(diff, diff.begin()));

        std::cout << std::endl << "Branches found in output tree but not in the reference tree:" << std::endl;
        for (const auto& b: diff) {
            std::cout << "  " << b << std::endl;
        }
    }
    CHECK(test_tree->GetNbranches() == n_branches);

    size_t entries = ref_tree->GetEntries();
    REQUIRE(test_tree->GetEntries() == entries);

    for (size_t entry = 0; entry < entries; entry++) {
        TObjArray* branches = ref_tree->GetListOfBranches();
        TIter next(branches);
        TBranch* ref_branch;
        bool all_branches_identical = true;
        while ((ref_branch = (TBranch*) next())) {
            ref_branch->GetEntry(entry);

            TBranch* test_branch = test_tree->GetBranch(ref_branch->GetName());
            CHECK(test_branch);
            if ( ! test_branch ) {
              all_branches_identical = false;
            } else {
              test_branch->GetEntry(entry);

              bool same_nBranches = (test_branch->GetListOfBranches()->GetEntries() == ref_branch->GetListOfBranches()->GetEntries()); CHECK(same_nBranches);

              bool same_diff = diffBranches(entry, ref_branch, test_branch);

              if ( ! ( same_nBranches && same_diff ) ) {
                all_branches_identical = false;
              }
            }
        }
        REQUIRE(all_branches_identical);
    }

    bool all_params_equal = true;
    if ( ref_file->GetListOfKeys()->GetSize() != test_file->GetListOfKeys()->GetSize() ) {
      std::cout << "Different number of keys: " << ref_file->GetListOfKeys()->GetSize() << " (ref) versus " << test_file->GetListOfKeys()->GetSize() << " (test)" << std::endl;
    }
    for ( const auto* iky : *(ref_file->GetListOfKeys()) ) {
      const auto& ky = *dynamic_cast<const TKey*>(iky);
      if ( std::string("TParameter<float>") == ky.GetClassName() ) {
        using floatparam = TParameter<float>;
        const float a = dynamic_cast<const floatparam*>(ref_file->Get(ky.GetName()))->GetVal();
        const auto bObj = test_file->Get(ky.GetName());
        if ( ! bObj ) {
          std::cout << "Error: key " << ky.GetName() << " not found in test file" << std::endl;
          all_params_equal = false;
        } else {
          const auto* bParam = dynamic_cast<const floatparam*>(bObj);
          if ( ! bParam ) {
            std::cout << "Error: object " << ky.GetName() << " in test file is not of type TParameter<float>" << std::endl;
            all_params_equal = false;
          } else {
            const float b = bParam->GetVal();
            if ( a != b ) {
              const auto reldiff = .5*std::abs(a-b)/(a+b);
              if ( reldiff < std::numeric_limits<float>::epsilon() ) {
                std::cout << "Warning: parameter " << ky.GetName() << " " << a << " vs " << b
                  << " rel.diff=" << reldiff << ", epsilon=" << std::numeric_limits<float>::epsilon() << std::endl;
              } else {
                all_params_equal = false;
              }
            }
          }
        }
      }
    }
    REQUIRE(all_params_equal);
}

int main(int argc, char* const argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <reference file> <test file>" << std::endl;
        return 1;
    }

    config.reference_file = argv[1];
    config.test_file = argv[2];

    Catch::Session session;
    return session.run();
}
