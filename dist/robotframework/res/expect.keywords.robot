*** Settings ***
Library           Process
Library           OperatingSystem
Library           String
Library           Collections

*** Variables ***
${RIOTTOOLS}            %{RIOTBASE}/dist/tools
${RIOTTESTS}            %{RIOTBASE}/tests

*** Keywords ***
Get Number of Lines Matching Pattern
    [Arguments]         ${pattern}
    ${lines} =          Get Lines Matching Pattern  ${RESULT.stdout}  ${pattern}
    ${count} =          Get Line Count  ${lines}
    [return]            ${count}

Get Sum From List
    [Arguments]         @{list}
    ${sum}=   Set Variable    0
    : FOR  ${elem}  IN   @{list}
    \    ${int}=    Convert To Integer  ${elem}
    \    ${sum}=    Evaluate            ${sum}+${int}
    [return]    ${sum}

Get Groups Matching Regexp
    [Arguments]         ${regex}
    #RESULT             CMD                 STING             REGEXP    GROUP
    @{groups}=          Get Regexp Matches  ${RESULT.stdout}  ${regex}  size
    [return]            @{groups}

Get First Group Matching Regexp
    [Arguments]         ${regex}
    @{groups}=          Get Groups Matching Regexp  ${regex}
    ${group}=           Set Variable  @{groups}[0]
    [return]            ${group}

Result Should Contain
    [Arguments]         ${pattern}
    Should Contain      ${RESULT.stdout}  ${pattern}

Verify Lines Matching Pattern Equal
    [Arguments]         ${pattern}  ${number}
    ${count} =          Get Number of Lines Matching Pattern  ${pattern}
    Should Be Equal As Integers  ${count}  ${number}

Verify Lines Matching Pattern Greater
    [Arguments]         ${pattern}  ${number}
    ${count} =          Get Number of Lines Matching Pattern  ${pattern}
    Should Be True      ${count} > ${number}
