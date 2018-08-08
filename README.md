# Framework
Common framework for all cp3-llbb analyses

*Please note:*
* The instructions are for the UCLouvain ingrid SLC6 cluster (to access SAMADhi)
* You need the proper username and password to access SAMADhi :) If you don't know what this is about, ask around
* The current state of the art mini-AOD documentation can be found [here](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2015)
* You will probably want to install as well [GridIn](https://github.com/cp3-llbb/GridIn) to run jobs on the grid, and one of the existing analyses ([TTAnalysis](https://github.com/cp3-llbb/TTAnalysis), [HHAnalysis](https://github.com/cp3-llbb/HHAnalysis), [ZAAnalysis](https://github.com/cp3-llbb/ZAAnalysis))

## CMSSW release and branches

We have different branches that contain the recipes and configuration for analysing different data samples.

Currently, there are a `CMSSW_8_0_6p` branch for the analysis of 2016 MiniAOD with the
[latest **CMSSW 8.0.X** release](https://raw.githubusercontent.com/cp3-llbb/Framework/CMSSW_8_0_6p/CMSSW.release),
and a `CMSSW_9_4_X` branch for analysis of 2016 2016 re-MiniAOD (and 2017 MiniAOD) with the
[latest **CMSSW 9.4.X** release](https://raw.githubusercontent.com/cp3-llbb/Framework/CMSSW_9_4_X/CMSSW.release).

## First time setup instructions

```bash
## the following two lines can be replaced by a call to the cms_env alias (see below)
source /nfs/soft/grid/ui_sl6/setup/grid-env.sh
source /cvmfs/cms.cern.ch/cmsset_default.sh

wget https://raw.githubusercontent.com/cp3-llbb/Framework/CMSSW_9_4_X/setup_project_with_framework.sh
source setup_project_with_framework.sh --branch BRANCH ## select a branch
```

[This script](setup_project_with_framework.sh) will set up a CMSSW release area,
apply the recipes in [``bootstrap_jenkins.sh``](bootstrap_jenkins.sh) and [``jenkins_postbuild.sh``](jenkins_postbuild.sh),
perform an initial build, and add your and your colleagues' forks on GitHub as remotes for your ``Framework`` clone.
The option `--branch NAME` can be used to select a branch (and CMSSW version, see above), and `--pr ID` to test a pull request.

If you are using ingrid, here's a useful alias to put in your ``bashrc`` file:

```bash
alias cms_env="module purge; module load grid/grid_environment_sl6; module load crab/crab3; module load cms/cmssw; module load slurm/slurm_utils;"
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
  * If you change anything to the output trees (new or modified branches, new recipes etc.), the automatic tests (see below) will fail, because they compare the outputs to reference files.
    You can resolve this by regenerating the reference files with the [`test/generate_reference_trees.sh`](test/generate_reference_trees.sh) script, after committing your other changes.
    It will also print a summary of all differences in the output files. If these are as expected, you can make a new commit with the updated reference files.

# Jenkins

When opening a new Pull Request, an automated tool, [Jenkins](https://jenkins-ci.org/), takes care of launching a full build. It allows to see directly if your code can be merged without breaking everything. We have a dedicated Jenkins instance running at CERN, accessible via https://jenkins-cp3-llbb.web.cern.ch/ ; Only members of the ``cp3-llbb`` CERN e-group can access this page.

As soon as a new Pull Request is opened, or if an already opened Pull Request is updated, an automatic build is launched. Only one build can be executed at the same time: every other builds are queued. A build typically takes about 1 hour.

Once a build is started, the Pull Request status on GitHub is updated. Once done, the status will either be green (the code compiles) or red (something is wrong). You can click on the ``Details`` link to access the Jenkins job report and the compilation log. For more information, see https://github.com/blog/1935-see-results-from-all-pull-request-status-checks

The Pull Request won't be mergeable until the Pull Request status is green.

## Bootstrap

**This part is very important**

Since the build is automatized, Jenkins needs to know how-to setup the CMSSW env by itself. To do that, two files are necessary:

 - ``CMSSW.release``: This file must contains only a string representing the CMSSW version to use to setup the framework. Be careful not to add a line break at the end of the line.
 - `` CMSSW.arch``: The ``SCRAM_ARCH`` of the CMSSW release.
 - ``bootstrap_jenkins.sh``: This file is a bash script executed by Jenkins just before building the framework, but after the CMSSW env is setup. You **must** use this file to install all the dependencies of the framework.
 - ``jenkins_postbuild.sh``: This file is executed by Jenkins after the compilation.

**Do not forget to update these files when changes are done to the release or the dependencies, otherwise the build will fail.**

## Technical details

 - Jenkins instance: https://jenkins-cp3-llbb.web.cern.ch/
 - [OpenShift](https://www.openshift.com/) instance: https://openshift.cern.ch/console/project/jenkins-cp3-llbb/overview (access is restricted to administrators. If you want / need access, please ask Christophe D. or Sébastien B.). This is the platform hosting our Jenkins instance inside an isolated container (for more information, look for Docker on Google).

A github bot also exists: https://github.com/cp3-llbb-bot ; it's a generic github user, member of the cp3-llbb organization. It needs push authorization to a repository to properly update the PR status. Password for this user can be found on the protected CP3 [wiki](https://cp3.irmp.ucl.ac.be/projects/cp3admin/wiki/UsersPage/Private/Physics/Exp/llbb)

**Note**: Sometimes, the container responsible for the build get stuck in creating phase and you'll need to kill it and retrigger a new build. To do that, connect to the OpenShift instance, and select on the left menu `Applications` → `Pods`. Click on the build instance in the list, and on the new page, select `Delete` in the `Actions` menu (top right). It may takes some time before the container is killed. If it's still stuck in deleting after a few minutes, you'll have to open a new ticket [here](https://cern.service-now.com/service-portal/service-element.do?name=PaaS-Web-App).

## Troubleshooting

  - If a build/test fails because of unexpected connection glitch, you can re-trigger jenkins by commenting `please test` to the pull request
