#!/bin/sh

# Copyright 2018 Sebastian Meiling <s@mlng.net>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

: "${RIOTBASE:=$(cd $(dirname $0)/../../; pwd)}"
: "${RFDIR:=${RIOTBASE}/dist/robotframework/}"
: "${RFOUTPUT:=${RIOTBASE}/robot/out/$(date +%Y%m%d_%H%M%S)}"

export RIOTBASE=${RIOTBASE}

[ -z "$BASE_BRANCH" ] && {
    echo "BASE_BRANCH not set default to \"master\"."
    BASE_BRANCH="master"
}

export BASE_BRANCH=${BASE_BRANCH}

cd ${RFDIR}

if [ -z "$1" ] ; then
    echo "usage: $0 test1.robot [test2.robot ...]"
    exit 1
fi

TESTS=$*
echo "TESTS: ${TESTS}"
robot --noncritical warn-if-failed --outputdir ${RFOUTPUT} --pythonpath "${RFDIR}/lib:${RFDIR}/res:${RIOTBASE}/dist/tests" ${TESTS}
