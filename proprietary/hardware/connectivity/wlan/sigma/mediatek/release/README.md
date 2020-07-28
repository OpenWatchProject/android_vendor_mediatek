# **Sigma Certification Test Tool**

## Introduction
Sigma Certification Test Tool is developed for MediaTek's products, according to [Wi-FiTestSuite-Linux-DUT](https://github.com/Wi-FiTestSuite/Wi-FiTestSuite-Linux-DUT), to support Wi-Fi Alliance certification program development and device certification including:

- STA
    - Wi-Fi CERTIFIED a/b/g/n
    - Wi-Fi CERTIFIED ac
    - Protected Management Frames
    - TDLS
    - Wi-Fi Direct
    - Voice-Enterprise
    - WMM-Admission Control
    - WMM-Power Save
    - Passpoint Release 2
    - Wi-Fi CERTIFIED WPA3
    - WPA2 security improvements
    - Wi-Fi Protected Setup
- AP
    - Wi-Fi CERTIFIED a/b/g/n
    - Wi-Fi CERTIFIED ac
    - Protected Management Frames
    - WMM-Power Save
    - WPA2 security improvements

## Environment setup
### General
1. Update the adb to the latest version on your PC/notebook at first.
2. The path of tool should not have space. For example,

                  (X) D:\PATH TO YOU SIGMA TOOL\sigma
                  (O) D:\PATH_TO_YOU_SIGMA_TOOL\sigma

3. Fix DUT's mac address in EngineerMode and fill the mac address in UCC init file. For example,

                  (STA) define!$DutMacAddress!00:08:22:11:49:15!
                  (AP)  define!$DutMacAddress_24G!00:90:4C:09:00:BD!
                        define!$DutMacAddress_5G!00:90:4c:13:00:BD!

4. Make sure Wi-Fi & Hotspot are turned off from Settings UI.
5. Update config in *scripts/config.txt*
-  Choose port for CA to listen. For example,

                  LISTEN_PORT=9000

-  Update DUT's ip and netmask. For example,

                  DUT_IP_ADDRESS=192.168.43.120
                  DUT_IP_NETMASK=255.255.255.0

- If CA and UCC were installed in the same computer, disable port forwarding. For example,

                  PORT_FORWARD=false

- Add tool name(e.g. route), which is supported by busybox, to list therefore you can use it like *data/bin/route*.

                  BUSYBOX=awk less more ipaddr arp route

### STA
1. TGn
- Put cas.pem, wifiuser.key and wifiuser.pem in *scripts/CertProgram/TGn*
2. WPA3
- Put cas.pem and wifiuser.pem in *scripts/CertProgram/WPA3*

### AP
1. Set IP address of control network on CA computer. For example,

                  IP: 192.168.250.99, netmask: 255.255.255.0

2. Update config in *scripts/config.txt*

                  PCENDPOINT_IP_ADDRESS=192.165.100.99
                  PCENDPOINT_IP_NETMASK=255.255.0.0

3. Update UCC init file

                  # ipaddr is the same as CA computer's, port is the same as LISTEN_PORT
                  wfa_control_agent_dut!ipaddr=192.168.250.99,port=9000!
                  wfa_console_ctrl!ipaddr=192.168.250.99,port=9000!

                  # the same as PCENDPOINT_IP_ADDRESS
                  dut_wireless_ip!192.165.100.99!
                  wfa_console_tg!192.165.100.99!

                  # the same as PCENDPOINT_IP_NETMASK
                  define!$netmask!255.255.0.0!

4. For overlapping test cases:
- TGn: 4.2.31, 4.2.32, 4.2.35
- TGac: 4.2.50

  Modify the test scripts to support two PC endpoints **(RECOMMANDED)**
  1. Refer to *scripts/CertProgram/AP/test_scripts* folder to modify the UCC test scripts to support two  PC endpoints.
  2. Set environment PCE's ip in UCC init file.

                  env_pce_control_agent_ip!ipaddr=192.168.250.30,port=9003!
                  env_pce_wireless_ip!192.165.100.35!

  or Bridge APUT's network.
  1. Turn on USB tethering from Settings on DUT
     - Create a bridge between data network and USB network on CA computer and set an available IP & net mask for this bridge network. For example,

                  IP: 192.165.100.162, netmask: 255.255.0.0

  2. Enable promiscuous mode on RNDIS interface on CA computer
     - List all bridge adapters and find RNDIS interface's id

                  $ netsh bridge show adapter

     - Enable promiscuous mode on RNDIS interface

                  $ netsh bridge set adapter 1 forcecompatmode=enable

     - Check promiscuous mode enabled on RNDIS interface

                  $ netsh bridge show adapter

## How to run
If you do this test firstly on this device, please:
1. **install_sigma.bat**: run this bat file to install all sigma running environments
- Select test mode according to the certification program under test:

  - STA
      - Wi-Fi CERTIFIED a/b/g/n => TGn
      - Wi-Fi CERTIFIED ac + Protected Management Frames => TGac
      - TDLS => TDLS
      - Wi-Fi Direct => P2P
      - Voice-Enterprise => VoE
      - WMM-Admission Control => WMMAC
      - WMM-Power Save => WMMPS
      - Passpoint Release 2 => PPR2
      - Wi-Fi CERTIFIED WPA3 + WPA2 security improvements => WPA3
      - Wi-Fi Protected Setup => WPS
  - AP => AP

When sigma tool has been installed on your device, you can:
1. **start_sigma.bat**: run this bat file to start sigma
- 3(or 2 if PORT_FORWARD=false) consoles, which represent WFA-DUT, Control-Agent and PFService, will run and redirect logs to *log*.
2. **stop_sigma.bat**: run this bat file to stop sigma
- if you meet some errors when run sigma tool or you need to stop sigma test temporarily to fetch logs, you can run stop_sigma.bat, and input **N** to leave the phone in sigma test mode, this will save your time when starting sigma again.

**Note:** the logs for WFA-DUT and Control-Agent will be saved to ca_log.txt and dut_log.txt, if you meet any issue when run sigma tool, please provide these two logs, sniffer log, UCC log, and any other logs you collected.
- *log/pull.bat*: pull MediaTek's logs from DUT
- *log/logcat.bat*: helper to logcat Android's main/system/event log