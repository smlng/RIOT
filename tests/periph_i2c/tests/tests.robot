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
    [Tags]  warn-if-failed
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

Check Write Ten Bytes to a Register
    ${result}=          I2C.Write Regs          data=${VAL_10}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Regs           leng=10
    Should Be Equal     ${result['data']}       ${VAL_10}

Check Read Byte
    ${result1}=         I2C.Read Reg
    Should Contain      ${result1['result']}    Success
    ${result2}=         I2C.Read Byte
    Should Be Equal     ${result1['data']}      ${result2['data']}

Check Read Bytes
    ${result1}=         I2C.Read Regs           leng=3
    Should Contain      ${result1['result']}    Success
    ${result2}=         I2C.Read Bytes          leng=3
    Should Be Equal     ${result1['data']}      ${result2['data']}

Check Write Byte
    ${result1}=         I2C.Read Reg
    Should Contain      ${result1['result']}    Success
    ${result}=          I2C.Write Byte          data=${I2C_ADDR}
    Should Contain      ${result['result']}     Success
    ${result2}=         I2C.Read Byte
    Should Be Equal     ${result1['data']}      ${result2['data']}

Check Write Bytes
    ${result}=          I2C.Write Bytes         data=${WRITE_VAL_3}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Regs           leng=3
    Should Be Equal     ${result['data']}       ${VAL_3}

Read Reg Should Fail
    ${result}=          I2C.Read Reg            addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Read Regs Should Fail
    ${result}=          I2C.Read Regs           addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Read Byte Should Fail
    ${result}=          I2C.Read Byte           addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Read Bytes Should Fail
    ${result}=          I2C.Read Bytes          addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Write Reg Should Fail
    ${result}=          I2C.Write Reg           addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Write Regs Should Fail
    ${result}=          I2C.Write Regs          addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Write Byte Should Fail
    ${result}=          I2C.Write Byte          addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Write Bytes Should Fail
    ${result}=          I2C.Write Bytes         addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error

Read Bytes Recover From Failed Read Reg
    ${result1}=         I2C.Read Reg
    Should Contain      ${result1['result']}    Success
    ${result}=          I2C.Read Reg            addr=${I2C_ADDR + 1}
    Should Contain      ${result['result']}     Error
    ${result2}=         I2C.Read Byte
    Should Be Equal     ${result1['data']}      ${result2['data']}

Read No Stop No Start Flag
    ${result1}=         I2C.Read Regs           leng=3
    Should Contain      ${result1['result']}    Success
    ${result}=          I2C.Read Byte           flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Success
    Should Be Equal     ${result1['data'][0]}   ${result['data'][0]}
    ${result}=          I2C.Read Byte           flag=${I2C_NOSTARTSTOP}
    Should Contain      ${result['result']}     Success
    Should Be Equal     ${result1['data'][1]}   ${result['data'][0]}
    ${result}=          I2C.Read Byte           flag=${I2C_NOSTART}
    Should Contain      ${result['result']}     Success
    Should Be Equal     ${result1['data'][2]}   ${result['data'][0]}

Write No Stop No Start Flag
    [Tags]  warn-if-failed
    ${result}=          I2C.Write Byte          data = ${I2C_UREG}      flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Write Byte          data = 99               flag=${I2C_NOSTARTSTOP}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Write Byte          data = 100              flag=${I2C_NOSTART}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Regs           leng=2
    Should Contain      ${result['result']}     Success
    Should Be Equal     ${result['data']}       [99, 100]

Repeated Start Read Should Timeout
    ${result}=          I2C.Read Byte           flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Byte           flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Timeout
    BPT.Reset DuT
    BPT.Reset MCU

Repeated Start Write Register
    [Tags]  warn-if-failed
    ${result}=          I2C.Write Byte          data= ${I2C_UREG}       flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Write Byte          data= ${VAL_1}          flag=${I2C_NOSTOP}
    Should Contain      ${result['result']}     Success
    ${result}=          I2C.Read Reg
    Should Contain      ${result['result']}     ${VAL_1}
