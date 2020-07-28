# bt signaling test
./boots -c dut

# ble signaling test for USB module
./boots -relay ttyUSB0

# ble signaling test for BT CoB platform
./boots -relay ttyGS2 &
    => Run it on platform
./boots ttyACM0 115200 -relay ttyUSB0 115200
    => Run it on NB/PC

# bt rf tx test
./boots -c rft -p xx -c xx -t xx

# bt rf rx test
./boots -c rfr -p xx -c xx -t xx -a xx:xx:xx:xx:xx:xx

# bt rf rx stop to get result
./boots -c rfr_stop -p xx -c xx -t xx -a xx:xx:xx:xx:xx:xx
