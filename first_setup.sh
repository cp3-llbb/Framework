#!/bin/bash

# Look out for conflicts between git and cmssw
if [ ! -f ${CMSSW_BASE}/src/.git/HEAD ];
then
    echo "You seem to be on Ingrid and CMSSW area appears not to be set up correctly. Check README carefully."
    echo
    return 1
fi
cd ${CMSSW_BASE}/src/cp3_llbb/Framework
# configure the origin repository
GITHUBUSERNAME=`git config user.github`
GITHUBUSERREMOTE=`git remote -v | grep upstream | awk '{print $2}' | head -n 1 | cut -d / -f 2`
git remote add origin git@github.com:${GITHUBUSERNAME}/${GITHUBUSERREMOTE}

# Add the remaining forks
git remote add OlivierBondu https://github.com/OlivierBondu/Framework.git
git remote add blinkseb https://github.com/blinkseb/Framework.git
git remote add delaere https://github.com/delaere/Framework.git
git remote add BrieucF https://github.com/BrieucF/Framework.git
git remote add swertz https://github.com/swertz/Framework.git
git remote add vidalm https://github.com/vidalm/Framework.git
git remote add acaudron https://github.com/acaudron/Framework.git
git remote add AlexandreMertens https://github.com/AlexandreMertens/Framework.git
git remote add mdelcourt https://github.com/mdelcourt/Framework.git
git remote add nadjieh https://github.com/nadjieh/Framework.git
git remote add pieterdavid https://github.com/pieterdavid/Framework.git
