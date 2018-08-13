*** Settings ***
Suite Setup         DUT Must Have I2C Test Application
Test Setup          Reset Application

Library             I2Cdevice   port=%{PORT}        WITH NAME  I2C
Library             BPTdevice   port=%{BPT_PORT}    WITH NAME  BPT

Resource            testutil.keywords.robot
Resource            expect.keywords.robot
Resource            periph.keywords.robot

*** Test Cases ***
Acquire and Release
    ${result}=          I2C.Acquire
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Release
    Should Contain      ${result['result']}     Success

Double Acquire Should Fail
    ${result}=          I2C.Acquire
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Acquire
    Should Not Contain  ${result['result']}     Success

Double Acquire with Reset Should Not Fail
    ${result}=          I2C.Acquire
    Should Contain      ${result['result']}     Success
    BPT.Reset MCU
    BPT.Reset DuT
    ${result}=          I2C.Acquire
    Should Contain  ${result['result']}         Success
