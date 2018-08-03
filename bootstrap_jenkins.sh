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

echo "---> Adding TreeWrapper"
git clone -o upstream https://github.com/blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper

echo "---> Adding phase2 EGamma"
git cms-merge-topic -u nsmith-:EgammaFromMultiCl_932v2
mkdir -p RecoEgamma && pushd RecoEgamma
git clone -b integrated git@github.com:nsmith-/Phase2InterimID.git
popd

echo "---> Running git cms-checkdeps"
git cms-checkdeps -a
