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

bool diffBranches(size_t entry, TBranch* ref, TBranch* test) {

    if (ref->GetListOfBranches()->GetEntries() > 0) {
	for (size_t n = 0; n < (size_t) ref->GetListOfBranches()->GetEntries(); n++) {
	    TBranch* ref_subbranch = (TBranch*) ref->GetListOfBranches()->UncheckedAt(n);
	    REQUIRE(ref_subbranch);

	    TBranch* test_subbranch = (TBranch*) test->GetListOfBranches()->UncheckedAt(n);
	    REQUIRE(test_subbranch);

	    REQUIRE(diffBranches(entry, ref_subbranch, test_subbranch));
	}

	return true;
    }

    INFO("[Event " << entry << "] Checking branches " << ref->GetName() << " of type " << ref->GetClassName());

    ref->GetEntry(entry);
    test->GetEntry(entry);

    REQUIRE(test->GetListOfLeaves()->GetEntries() == ref->GetListOfLeaves()->GetEntries());

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

    REQUIRE(ref_sha == test_sha);

    return true;
}

TEST_CASE("Check if trees are equals", "[diff]") {
    std::unique_ptr<TFile> ref_file(TFile::Open(config.reference_file.c_str(), "read"));
    std::unique_ptr<TFile> test_file(TFile::Open(config.test_file.c_str(), "read"));

    REQUIRE(ref_file.get());
    REQUIRE(test_file.get());

    TTree* ref_tree = (TTree*) ref_file->Get("t");
    REQUIRE(ref_tree);

    TTree* test_tree = (TTree*) test_file->Get("t");
    REQUIRE(test_tree);

    ref_tree->SetBranchStatus("*", 1);
    test_tree->SetBranchStatus("*", 1);

    size_t n_branches = ref_tree->GetNbranches();
    if ((size_t) test_tree->GetNbranches() != n_branches) {
        std::vector<std::string> ref_branches;
        TObjArray* branches = ref_tree->GetListOfBranches();
        TIter next(branches);
        TBranch* branch;
        while ((branch = (TBranch*) next())) {
            ref_branches.push_back(branch->GetName());
        }

        std::vector<std::string> test_branches;
        branches = test_tree->GetListOfBranches();
        next = TIter(branches);
        while ((branch = (TBranch*) next())) {
            test_branches.push_back(branch->GetName());
        }

        std::vector<std::string> diff;
        std::set_difference(ref_branches.begin(), ref_branches.end(),
                test_branches.begin(), test_branches.end(),
                std::back_inserter(diff));

        std::cout << "Branches found in reference tree but not in output tree:" << std::endl;
        for (const auto& b: diff) {
            std::cout << "  " << b << std::endl;
        }

        diff.clear();
        std::set_difference(test_branches.begin(), test_branches.end(),
                ref_branches.begin(), ref_branches.end(),
                std::back_inserter(diff));

        std::cout << std::endl << "Branches found in output tree but not in the reference tree:" << std::endl;
        for (const auto& b: diff) {
            std::cout << "  " << b << std::endl;
        }
    }
    REQUIRE(test_tree->GetNbranches() == n_branches);

    size_t entries = ref_tree->GetEntries();
    REQUIRE(test_tree->GetEntries() == entries);

    for (size_t entry = 0; entry < entries; entry++) {
        TObjArray* branches = ref_tree->GetListOfBranches();
        TIter next(branches);
        TBranch* ref_branch;
        while ((ref_branch = (TBranch*) next())) {
            ref_branch->GetEntry(entry);

            TBranch* test_branch = test_tree->GetBranch(ref_branch->GetName());
            REQUIRE(test_branch);

            test_branch->GetEntry(entry);

            REQUIRE(test_branch->GetListOfBranches()->GetEntries() == ref_branch->GetListOfBranches()->GetEntries());

            REQUIRE(diffBranches(entry, ref_branch, test_branch));
        }
    }
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
