import socket
import time

TEST_ALL = True
TEST_GET_INFO = False
TEST_IP_CONFIG = False
TEST_ENCRYPTION = False
TEST_PSK = False
TEST_EAPTLS = False
TEST_EAPTTLS = False
TEST_EAPSIM = False
TEST_EAPPEAP = False
TEST_EAPAKA = False
TEST_RFEATURE = False
TEST_WMM = False
TEST_CONNECT = False
TEST_TRAFIC = False
TEST_WPA3 = False
TEST_SCAN = False
TEST_TIME = False

SOCKET_TIMEOUT = 120

HOST = '127.0.0.1'
PORT = 9000

total = 0
fail = 0
complete = 0

def send_resp(s, cmd):
    print '==============================='
    print cmd

    global total
    global fail
    global complete


    total += 1
    s.send(cmd + ",   ")
    response = s.recv(4096)
    print response

    if "COMPLETE" not in response:
        response = s.recv(4096)
        print response

    if "COMPLETE" in response:
        complete += 1
    else:
        fail += 1
    return response

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.settimeout(SOCKET_TIMEOUT)
client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
client_socket.connect((HOST, PORT))

if TEST_ALL or TEST_GET_INFO:
    send_resp(client_socket, "ca_get_version")
    send_resp(client_socket, "device_get_info")
    send_resp(client_socket, "device_get_info,firmware")
    send_resp(client_socket, "device_list_interfaces,interfaceType,802.11")
    send_resp(client_socket, "sta_get_info,wlan0")
    send_resp(client_socket, "sta_get_mac_address,interface,wlan0")
    send_resp(client_socket, "sta_get_bssid,interface,wlan0")
    send_resp(client_socket, "sta_set_uapsd,interface,wlan0,ssid,wifi,maxSPLength,2,acBK,0")

if TEST_ALL or TEST_IP_CONFIG:
    send_resp(client_socket, "sta_get_ip_config,interface,wlan0")
    send_resp(client_socket, "sta_set_ip_config,interface,wlan0,dhcp,0,ip,192.168.1.101,mask,255.255.255.0")
    send_resp(client_socket, "sta_set_ip_config,interface,wlan0,dhcp,1")

if TEST_ALL or TEST_CONNECT or TEST_TRAFIC:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_is_connected,interface,wlan0")
    send_resp(client_socket, "sta_get_bssid,interface,wlan0")
    send_resp(client_socket, "sta_set_psk,interface,wlan0,ssid,Hermaniphone,passPhrase,123456789,keyMgmtType,wpa2,encpType,aes-ccmp")
    send_resp(client_socket, "sta_associate,interface,wlan0,ssid,Hermaniphone")
    send_resp(client_socket, "sta_is_connected,interface,wlan0")
    send_resp(client_socket, "sta_get_bssid,interface,wlan0")
    send_resp(client_socket, "sta_get_ip_config,interface,wlan0")

    if TEST_TRAFIC:
        resp = send_resp(client_socket, "traffic_send_ping,destination,172.0.0.1,frameSize,1000,frameRate,2,duration,10")
        time.sleep(3)
        streamID = resp.split(",")[-1]
        send_resp(client_socket, "traffic_stop_ping,streamID,%s" % streamID)

    send_resp(client_socket, "sta_reassociate,interface,wlan0,ssid,Hermaniphone,bssid,f2:99:b6:21:a8:02")

    if TEST_TRAFIC:
        resp = send_resp(client_socket, "traffic_send_ping,destination,172.0.0.1,frameSize,1000,frameRate,2,duration,10")
        time.sleep(10)
        streamID = resp.split(",")[-1]
        send_resp(client_socket, "traffic_stop_ping,streamID,%s" % streamID)

    send_resp(client_socket, "sta_disconnect,interface,wlan0")
    send_resp(client_socket, "sta_is_connected,interface,wlan0")


if TEST_ALL or TEST_ENCRYPTION:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,PMF")
    send_resp(client_socket, "sta_set_encryption,interface,wlan0,ssid,wifi,encpType,wep,key1,12345678901234567890123456")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_encryption,interface,wlan0,ssid,wifi,encpType,none")

if TEST_ALL or TEST_PSK:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_psk,interface,wlan0,ssid,MyNetwork,passPhrase,123456789,keyMgmtType,wpa2,encpType,aes-ccmp")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_psk,interface,wlan0,ssid,MyNetwork,passphrase,9876543210,keymgmttype,wpa2,encpType,aes-ccmp,prog,hs2,prefer,1")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_psk,interface,wlan0,ssid,PcachePc,passphrase,PcachePc,encpType,aes-ccmp-tkip,keymgmttype,wpa2-wpa-psk")

if TEST_ALL or TEST_EAPTLS:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_eaptls,interface,wlan0,ssid,MyNetwork,trustedRootCA,Azimuth CA,clientCertificate,console-cert,encpType,tkip,keyMgmtType,wpa")

if TEST_ALL or TEST_EAPTTLS:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_eapttls,interface,wlan0,ssid,MyNetwork,username,test,password,test123,trustedRootCA,cas,encpType,tkip,keyMgmtType,wpa")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_eapttls,interface,wlan0,ssid,MyNetwork,username,test,password,test123,trustedRootCA,cas,encpType,aes-ccmp,keymgmttype,wpa2,prefer,1,prog,hs2")

if TEST_ALL or TEST_EAPSIM:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_eapsim,interface,wlan0,ssid,MyNetwork,username,console,password,azimuth,encpType,tkip,keyMgmtType,wpa")

if TEST_ALL or TEST_EAPPEAP:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_peap,interface,wlan0,ssid,MyNetwork,username,console,password,azimuth,trustedRootCA,Azimuth CA,encpType,tkip,keyMgmtType,wpa,innerEAP,MSCHAPv2,peapVersion,0")

if TEST_ALL or TEST_EAPAKA:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_eapaka,interface,wlan0,ssid,MyNetwork,username,console,password,azimuth,encpType,tkip,keyMgmtType,wpa")

if TEST_ALL or TEST_RFEATURE:
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,uapsd,enable")
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,uapsd,enable,peer,00:01:02:03:04:05")
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,uapsd,disable,peer,00:01:02:03:04:05")
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,tpktimer,disable")
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,ChSwitchMode,Enable,OffChNum,60,SecChOffset,40above")
    send_resp(client_socket, "sta_set_rfeature,interface,wlan0,prog,TDLS,ChSwitchMode,Disable")

if TEST_ALL or TEST_WMM:
    send_resp(client_socket, "sta_set_wmm,interface,wlan0,GROUP,wmmac,ACTION,addts,DIALOG_TOKEN,10,TID,2,DIRECTION,uplink,PSB,UAPSD,UP,7,Fixed,true,SIZE,208,MAXSIZE,512,MIN_SRVC_INTRVL,3000,MAX_SRVC_INTRVL,3001,INACTIVITY,1000,SUSPENSION,20000,SRVCSTARTTIME,300,MINDATARATE,83200,MEANDATARATE,80000,PEAKDATARATE,83201,PHYRATE,6000000,BURSTSIZE,100000,DELAYBOUND,12100,SBA,1.5,MEDIUM_TIME,3567,ACCESSCAT,VO")
    send_resp(client_socket, "sta_set_wmm,interface,wlan0,GROUP,wmmac,ACTION,delts,TID,2")

if TEST_ALL or TEST_WPA3:
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_security,type,eaptls,interface,wlan0,ssid,MyNetwork,trustedRootCA,Azimuth CA,clientCertificate,console-cert,encpType,tkip,keyMgmtType,wpa,pmf,enable")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_security,Interface,wlan0,profile,1,SSID,SSID-absent1,Type,PSK,KeyMgmtType,WPA2,EncpType,AES-CCMP,passphrase,1234512345")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    # FILS-SHA256 not supported
    #send_resp(client_socket, "sta_set_security,interface,wlan0,profile,1,type,EAPTTLS,ssid,OCE-test,username,ttls-user@wfa.oce.test,password,password,trustedRootCA,cas.pem,KeyMgmtType,WPA2,EncpType,AES-CCMP,AKMSuiteType,14,PMKSACaching,disabled")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_security,type,SAE,interface,wlan0,ssid,MyNetwork,KeyMgmtType,WPA2,EncpType,AES-CCMP,passphrase,12345678")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    # WPA-EAP-SUITE-B-192 not supported
    #send_resp(client_socket, "sta_set_security,type,eaptls,interface,wlan0,ssid,MyNetwork,usernametest,trustedRootCA,Azimuth CA,clientCertificate,console-cert,keymgmtType,SuiteB,PairwiseCipher,AES-GCMP-256,GroupCipher,AES-GCMP-256,GroupMgntCipher,BIP-GCMP-256")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_security,type,open,interface,wlan0,ssid,MyNetwork")
    send_resp(client_socket, "sta_reset_default,interface,wlan0,prog,VHT")
    send_resp(client_socket, "sta_set_security,type,owe,interface,wlan0,ssid,MyNetwork,usernametest,trustedRootCA,Azimuth CA,clientCertificate,console-cert,keymgmtType,SuiteB,PairwiseCipher,AES-GCMP-256,GroupCipher,AES-GCMP-256,GroupMgntCipher,BIP-GCMP-256")

if TEST_ALL or TEST_SCAN:
    send_resp(client_socket, "sta_scan,interface,wlan0")
    time.sleep(10)
    send_resp(client_socket, "sta_scan,interface,wlan0,ssid,CS9,bssid,00:01:02:03:04:05")

if TEST_ALL or TEST_TIME:
    send_resp(client_socket, "sta_set_systime,month,04,date,01,year,2036,hours,0,minutes,0,seconds,0")

print '==============================='
print "Total:%d, Complete:%d, Fail:%d" % (total, complete, fail)