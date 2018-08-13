*** Keywords ***
DUT Must Have I2C Test Application
    ${result}=          I2C.Get ID
    Should Contain      ${result['result']}     Success
    Should Contain      ${result['msg']}        periph_i2c
