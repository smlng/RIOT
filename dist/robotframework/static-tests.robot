*** Settings ***
Documentation       A suite of static code tests
...
...                 These tests are typically run before any other steps, like
...                 compiling, linking, and running functional tests.

Suite Teardown      Terminate All Processes    kill=True
Resource            testutil.keywords.robot

*** Test Cases ***
toolchains
    [Tags]  core  warn-if-failed
    Run Static Test     ${RIOTTOOLS}/ci/print_toolchain_versions.sh

coccinelle
    [Tags]  core
    Run Static Test

cppcheck
    [Tags]  core
    Run Static Test

externc
    [Tags]  core
    Run Static Test

flake8
    [Tags]  core  warn-if-failed
    Run Static Test

headerguards
    [Tags]  core
    Run Static Test

whitespacecheck
    [Tags]  core
    Run Static Test
