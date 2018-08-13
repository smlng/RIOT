*** Settings ***
Test Setup          Binary Should Exist         xtimer_reset
Test Teardown       Terminate All Processes     kill=True

Resource            testutil.keywords.robot
Resource            expect.keywords.robot

*** Test Cases ***
xtimer_hang
    [Documentation]     Try re-setting of an already active timer.
    [Tags]              xtimer
    Run Test            application=xtimer_reset     timeout=20 secs
    # keyword                               # pattern               # == number
    Verify Lines Matching Pattern Equal     *now=[0123456789]*      3
    Result Should Contain   Test completed!
