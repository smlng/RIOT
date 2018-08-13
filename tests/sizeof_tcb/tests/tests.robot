*** Settings ***
Suite Setup         Binary Should Exist
Test Setup          Reset Application
Test Teardown       Terminate All Processes     kill=True

Resource            testutil.keywords.robot
Resource            expect.keywords.robot

Variables           test_vars.py

*** Test Cases ***
Verify Thread_t Size Simple
    [Documentation]     Verify the size of thread_t by comparing strings
    [Tags]              core
    Run Test            timeout=10 secs
    ${tcb_size}=        Get First Group Matching Regexp  ${REGEXP_TCB_TOTAL}
    ${tcb_parts}=       Set Variable If  ${tcb_size} > 30  ${TCB_PARTS_32}  ${TCB_PARTS_16}
    : FOR  ${elem}  IN   @{tcb_parts}
    \                   Result Should Contain  ${elem}
    Result Should Contain  SUCCESS

Verify Thread_t Size Sum
    [Documentation]     Verify the size of thread_t by calculating sum
    [Tags]              core
    Run Test            timeout=10 secs
    ${tcb_size}=        Get First Group Matching Regexp  ${REGEXP_TCB_TOTAL}
    @{tcb_parts}=       Get Groups Matching Regexp       ${REGEXP_TCB_PARTS}
    ${tcb_sum}=         Get Sum from List   @{tcb_parts}
    Should Be Equal As Integers         ${tcb_size}     ${tcb_sum}
    Result Should Contain  SUCCESS
