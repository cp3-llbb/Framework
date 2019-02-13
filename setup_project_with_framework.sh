#! /bin/sh

# Bootstrap a CMSSW installation ready for a given framework version
# Usage: source setup_project_with_framework.sh [-j N] [-o NAME] [ -b BRANCH || --pr PULLREQUESTID ]

FRAMEWORK_GITHUB=https://github.com/cp3-llbb/Framework.git

# Default options
CORE=4
OUTPUT=""

# Parse command line
while [[ $# > 0 ]]
do
    key="$1"

    case $key in
        -j|--core)
            CORE="$2"
            shift # past argument
            ;;
        -o|--output)
            OUTPUT="$2"
            shift # past argument
            ;;
        --pr)
            GITREF="refs/pull/$2/head"
            BRANCH="test-PR$2"
            shift # past argument
            ;;
        -b|--branch)
            GITREF="$2"
            BRANCH="$2"
            shift # past argument
            ;;
        -a|--arch)
            CMSSW_ARCH="$2"
            shift
            ;;
        -r|--release)
            CMSSW_VERSION="$2"
            shift
            ;;
        -d|--bootstrap)
            DEPENDENCIES="$2"
            shift
            ;;
        *)
            # unknown option
            echo "Usage: source setup_project_with_framework.sh [-j N] [-o NAME] [ -b BRANCH OR --pr PULLREQUESTID ] [-a ARC] [-r CMSSW_X_Y_Z] [-b dependencies.sh]"
            return 1
            ;;
    esac
    shift # past argument or value
done

## for messages
if [[ "${BRANCH}" == "${GITREF}" ]]; then
  GITDESCR="branch ${BRANCH}"
else
  GITDESCR="ref ${GITREF}:${BRANCH}"
fi

## Clone into a temporary directory, to make sure we get the correct architecture and release
TEMPDIR=$(mktemp -d)
FWKDIR_TMP="${TEMPDIR}/Framework"
git clone -o upstream "${FRAMEWORK_GITHUB}" "${FWKDIR_TMP}"
if [ -n "${GITREF}" ]; then
  pushd "${FWKDIR_TMP}" &> /dev/null
  ( git fetch upstream "${GITREF}:${BRANCH}" && git checkout "${BRANCH}" ) || ( echo "Problem checking out ${GITDESCR}" && exit 1 ) || return 1
  popd &> /dev/null
fi

if [ -z "${CMSSW_ARCH}" ]; then
  CMSSW_ARCH=$(cat "${FWKDIR_TMP}/CMSSW.arch")
fi
if [ -z "${CMSSW_VERSION}" ]; then
  CMSSW_VERSION=$(cat "${FWKDIR_TMP}/CMSSW.release")
fi
if [ -z "${DEPENDENCIES}" ]; then
  DEPENDENCIES="${FWKDIR_TMP}/bootstrap_jenkins.sh"
fi

if [ -z "${OUTPUT}" ]; then
  OUTPUT="${CMSSW_VERSION}"
fi

echo ""
echo "Setting up ${CMSSW_VERSION} (${CMSSW_ARCH}) for Framework ${GITDESCR} into ${OUTPUT}"
echo ""

# Setup CMS env if necessary
if [ -z "${CMS_PATH}" ]; then
  source /cvmfs/cms.cern.ch/cmsset_default.sh
fi

scramv1 -a "${CMSSW_ARCH}" project -n "${OUTPUT}" CMSSW "${CMSSW_VERSION}"
pushd "${OUTPUT}/src" &> /dev/null
eval `scramv1 runtime -sh` # cmsenv

## install dependencies
source "${DEPENDENCIES}"

## move Framework to the right place
mkdir -p "cp3_llbb"
mv "${FWKDIR_TMP}" "cp3_llbb/"
rmdir "${TEMPDIR}"

# Configure git remotes
pushd "cp3_llbb/Framework" &> /dev/null
scripts/updateremotes
popd &> /dev/null

echo ""
echo "Bootstrapping done. Launching build using ${CORE} cores"
echo ""

## compile
scram b -j${CORE}

## check out a few more
source "cp3_llbb/Framework/jenkins_postbuild.sh"

OUTPUT_FULL=$(pwd) ## absolute path, for final message
popd &> /dev/null

echo ""
echo "All done, your project area in ${OUTPUT_FULL} is ready"
