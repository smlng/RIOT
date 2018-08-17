*** Settings ***
Library     Process
Library     OperatingSystem
Library     String
Library     Collections

*** Variables ***
${RIOTTOOLS}            %{RIOTBASE}/dist/tools
${RIOTTESTS}            %{RIOTBASE}/tests

*** Keywords ***
Binary Should Exist
    File Should Exist   %{ELFFILE}

Reset Application
    Run Process         make reset  shell=True  cwd=%{APPDIR}
    Sleep               5

Run Test
    [Arguments]         ${timeout}=30 secs
    ${ptest}=           Start Process       make term  shell=True  cwd=%{APPDIR}
    Process Should Be Running  ${ptest}
    Reset Application
    Wait For Process    handle=${ptest}  timeout=${timeout}  on_timeout=kill
    ${RESULT} =         Get Process Result
    Log Many            stdout: ${RESULT.stdout}    stderr: ${RESULT.stderr}
    Set Test Variable   ${RESULT}

Run Static Test
    [Arguments]         ${static_test}=${RIOTTOOLS}/${TEST NAME}/check.sh
    ${result} =         Run Process    ${static_test}
    Log Many            stdout: ${result.stdout}    stderr: ${result.stderr}
    Should Be Equal As Integers    ${result.rc}    0
