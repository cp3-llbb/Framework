# Framework
Common framework for all cp3-llbb analyses

*Please note:*
* The instructions are for the UCLouvain ingrid SLC6 cluster (to access SAMADhi)
* You need the proper username and password to access SAMADhi :) If you don't know what this is about, ask around
* The current state of the art mini-AOD documentation can be found [here](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2015)
* You will probably want to install as well [GridIn](https://github.com/cp3-llbb/GridIn) to run jobs on the grid, and one of the existing analyses ([TTAnalysis](https://github.com/cp3-llbb/TTAnalysis), [HHAnalysis](https://github.com/cp3-llbb/HHAnalysis), [ZAAnalysis](https://github.com/cp3-llbb/ZAAnalysis))

## CMSSW release

**CMSSW_7_6_5**

## First time setup instructions

```bash
source /nfs/soft/grid/ui_sl6/setup/grid-env.sh
source /cvmfs/cms.cern.ch/cmsset_default.sh
export SCRAM_ARCH=slc6_amd64_gcc493
cmsrel CMSSW_7_6_5
cd CMSSW_7_6_5/src
cmsenv

# JER stuff
git cms-merge-topic blinkseb:smeared_jet_producer

# CP3-llbb framework itself
git clone -o upstream git@github.com:blinkseb/TreeWrapper.git cp3_llbb/TreeWrapper
git clone -b CMSSW_7_6_3p -o upstream git@github.com:cp3-llbb/Framework.git cp3_llbb/Framework

scram b -j 4

cd ${CMSSW_BASE}/src/cp3_llbb/Framework
source first_setup.sh
cd ${CMSSW_BASE}/src
```

If you are using ingrid, here's a useful alias to put in your ``bashrc`` file:

```bash
alias cms_env="module purge; module load grid/grid_environment_sl6; module load crab/crab3; module load cms/cmssw;"
```

Then, just do ``cms_env`` to load all the CMSSW environment.

## Test run (command line)

```bash
cd ${CMSSW_BASE}/src/cp3_llbb/Framework/test
cmsRun TestConfigurationMC.py
```

# When willing to commit things
  * Remember to *branch before committing anything*: ```git checkout -b my-new-branch```
  * The ```first_setup.sh``` script took care of adding ```origin``` as your own repo, so to push just do the usual ```git push origin my-new-branch```

# Jenkins

When opening a new Pull Request, an automated tool, [Jenkins](https://jenkins-ci.org/), takes care of launching a full build. It allows to see directly if your code can be merged without breaking everything. We have a dedicated Jenkins instance running at CERN, accessible via https://jenkins.cern.ch/cp3-llbb/ ; Only members of the ``cp3-llbb`` CERN e-group can access this page.

Jenkins runs every 5 minutes to check if there's something new. If a new Pull Request is detected, or if an already opened Pull Request is updated, an automatic build is launched. Only one build can be executed at the same time: every other builds are queued. A build typically takes about 1 hour.

Once a build is started, the Pull Request status on GitHub is updated. Once done, the status will either be green (the code compiles) or red (something is wrong). You can click on the ``Details`` link to access the Jenkins job report and the compilation log. For more information, see https://github.com/blog/1935-see-results-from-all-pull-request-status-checks

The Pull Request won't be mergeable until the Pull Request status is green.

## Bootstrap

**This part is very important**

Since the build is automatized, Jenkins needs to know how-to setup the CMSSW env by itself. To do that, two files are necessary:

 - ``CMSSW.release``: This file must contains only a string representing the CMSSW version to use to setup the framework. Be careful not to add a line break at the end of the line.
 - `` CMSSW.arch``: The ``SCRAM_ARCH`` of the CMSSW release.
 - ``bootstrap_jenkins.sh``: This file is a bash script executed by Jenkins just before building the framework, but after the CMSSW env is setup. You **must** use this file to install all the dependencies of the framework.

**Do not forget to update these files when changes are done to the release or the dependencies, otherwise the build will fails.**

## Technical details

 - Jenkins instance: https://jenkins.cern.ch/info/cp3-llbb
 - Jenkins instance informations: https://jenkins.cern.ch/info/cp3-llbb
 - CERN forge: https://cernforge.cern.ch/jenkins/details/cp3-llbb (only accessible by the Administrator, Sébastien B.)

The builds are done on a custom VM hosted on CERN OpenStack servers, because Jenkins instance does not have access to either AFS or CVMFS. The VM instance is accessible only from CERN network:

 - Hostname: cp3-llbb-buildbot
 - SSH access only via key pair. Only accessible by the Administrator, Sébastien B.

A github bot also exists: https://github.com/cp3-llbb-bot ; it's a generic github user, member of the cp3-llbb organization. It needs push authorization to a repository to properly update the PR status. Password for this user can be found on the protected CP3 [wiki](https://cp3.irmp.ucl.ac.be/projects/cp3admin/wiki/UsersPage/Private/Physics/Exp/llbb)

## Troubleshooting

  - If a build/test fails because of unexpected connection glitch, you can re-trigger jenkins by commenting `test this please` to the pull request
