*** Settings ***
Test Setup          Binary Should Exist         xtimer_hang
Test Teardown       Terminate All Processes     kill=True

Resource            testutil.keywords.robot
Resource            expect.keywords.robot

*** Test Cases ***
xtimer_hang
    [Documentation]     runtime test to check if xtimer runs into a deadlock, it
    ...                 is not about clock stability nor timing accuracy.
    [Tags]              xtimer
    Run Test            application=xtimer_hang     timeout=20 secs
    # keyword                               # pattern               # > number
    Verify Lines Matching Pattern Greater   *Testing (???%)         90
    Result Should Contain   SUCCESS
