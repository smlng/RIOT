*** Settings ***
Suite Setup         DUT Must Have I2C Test Application
Test Setup          Reset DUT and BPT

Library             I2Cdevice   port=%{PORT}        WITH NAME  I2C
Library             BPTdevice   port=%{BPT_PORT}    WITH NAME  BPT

Resource            testutil.keywords.robot
Resource            expect.keywords.robot
Resource            periph.keywords.robot

Variables           test_vars.py

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
    BPT.Reset DuT
    ${result}=          I2C.Acquire
    Should Contain      ${result['result']}     Success

Check Write One Byte to a Register
    ${result}=          I2C.Write Reg           data=${VAL_1}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Reg
    Should Be Equal     ${result['data']}       ${VAL_1}

Check Write Two Bytes to a Register
    ${result}=          I2C.Write Regs          data=${VAL_2}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Regs           leng=2
    Should Be Equal     ${result['data']}       ${VAL_2}
