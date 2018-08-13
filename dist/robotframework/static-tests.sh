#!/bin/sh

# Copyright 2018 Sebastian Meiling <s@mlng.net>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

: "${RIOTBASE:=$(cd $(dirname $0)/../../; pwd)}"
: "${RFBASE:=${RIOTBASE}/dist/robotframework/}"
: "${RFOUTPUT:=${RIOTBASE}/robot/static-tests/$(date +%Y%m%d_%H%M%S)}"

export RIOTBASE=${RIOTBASE}

[ -z "$BASE_BRANCH" ] && {
    echo "BASE_BRANCH not set default to \"master\"."
    BASE_BRANCH="origin/master"
}

export BASE_BRANCH=${BASE_BRANCH}

cd ${RFBASE}

robot --noncritical warn-if-failed -P "${RFBASE}/lib:${RFBASE}/res" -d ${RFOUTPUT} static-tests.robot
