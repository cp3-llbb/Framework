#!/usr/bin/env bash

function cms-merge-commit()
{
  local github_user="${1}"
  local topic_branch="${2}"
  local commit_hash="${3}"
  echo "-----> Merging commit ${commit_hash} on branch ${topic_branch} from ${github_user}"

  git remote add -t "${topic_branch}" "${github_user}" "https://github.com/${github_user}/cmssw.git"
  git fetch --no-tags "${github_user}" "${topic_branch}:refs/remotes/${github_user}/${topic_branch}" || { echo "Could not fetch branch ${topic_branch} from ${github_user}"; exit 1; }
  local current_branch="$(git rev-parse --abbrev-ref HEAD)"
  local merge_base="$(git merge-base ${commit_hash} ${current_branch})"
  git cms-sparse-checkout "${merge_base}" "${commit_hash}"
  git read-tree -mu HEAD
  git merge --no-ff -m "Merged ${commit_hash} on branch ${topic_branch} from ${github_user} into ${current_branch}" "${commit_hash}" || { echo "Could not merge ${commit_hash} on branch ${topic_branch} from ${github_user} into ${current_branch}"; exit 1; }
}

# CMSSW env is already configured
# Current working dir is $CMSSW_BASE/src

git cms-init --upstream-only

echo "---> Merging recipes..."
cms-merge-commit cms-egamma EGM_gain_v1 f2ae5ef247d2544bdccd0460586b468eb35631c5
cms-merge-commit rafaellopesdesa RegressionCheckNegEnergy 3aafeff0371a1d1eb3db9d95ef50c1a66da25690
cms-merge-commit cms-met METRecipe_8020_for80Xintegration 92f73cd3d16a9529585865a365de271e0535b68d

echo "---> Running git cms-checkdeps"
git cms-checkdeps -a

echo "---> Adding KaMuCa"
git clone -o upstream https://github.com/bachtis/analysis.git -b KaMuCa_V4 KaMuCa 
pushd KaMuCa
git checkout 2ad38daae37a41a9c07f482e95f2455e3eb915b0
popd

echo "---> Adding TreeWrapper"
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
