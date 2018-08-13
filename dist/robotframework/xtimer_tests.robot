*** Settings ***
Documentation       A test suite for the RIOT xtimer implementation
...
...                 Runs a selection of timer tests

Suite Teardown      Terminate All Processes    kill=True
Test Setup          Build And Flash    ${TEST NAME}

Resource            testutil.keywords.robot
Resource            expect.keywords.robot

*** Test Cases ***
xtimer_hang
    Run Test    timeout=20 secs
    # keyword                               # pattern               # > number
    Verify Lines Matching Pattern Greater   *Testing (???%)         90
    Result Should Contain   SUCCESS

xtimer_reset
    Run Test    timeout=10 secs
    # keyword                               # pattern               # == number
    Verify Lines Matching Pattern Equal     *now=[0123456789]*      3
    Result Should Contain   Test completed!

xtimer_remove
    Run Test    timeout=10 secs
    Verify Lines Matching Pattern Equal     *Setting 3 timers, removing timer ?/3  3
    Result Should Contain   test successful.

xtimer_usleep_short
    Run Test    timeout=25 secs
    Verify Lines Matching Pattern Equal     *going to sleep * usecs...  501
    Result Should Contain   SUCCESS
