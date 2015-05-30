/*
 * TrendChip Command Interpreter for ASUS-DSL routers
 * Version 0.1, by cav94mat.
 */
 
#ifndef TC_VERSION 
 /** Constants **/
 /* asus-tcci */
    #define AUTHOR             "cav94mat"
    #define URL                "https://github.com/cav94mat/asus-tcci/"
    #define VERSION            "0.1"
    // Exit codes
    #define EC_NORMAL           0
    #define EC_NETFAIL          1
    #define EC_APIFAIL          2
    // Defaults
    #define DEF_ADAPTER         "eth2.1"    /* -a */
    #define DEF_ADAPTER_REMOTE  "eth2"      /* -b */
    #define DEF_CMD_INIT        "sys ver"   /* (operand) */
 /* Ethernet */
    #define MAC_LEN             6
    #define ETH_MAX_SIZE        1600
    #define ETH_TYPE_LEN        2
 /* TCCI-over-Ethernet */
    #define TC_ETHF_REQUEST     6 // MAC_RTS_CONSOLE_CMD
    #define TC_CMD_MAX_LEN      256
    #define TC_ETH_TYPE         "\xaa\xaa"
    #define TC_LF               "\x0d\x0a"
    /*
    #define MAC_RTS_START 1
    #define MAC_RTS_RESPONSE 2
    #define MAC_RTS_STOP 3
    #define MAC_RTS_ON 4
    #define MAC_RTS_CONSOLE_ON 5
    #define MAC_RTS_CONSOLE_CMD 6
    #define MAC_RTS_CONSOLE_DATA 7
    */
 /* Obsolete? */
    #define MAX_RESP_BUF_NUM    24
    #define MAX_RESP_BUF_SIZE   256
    #define MAX_TC_RESP_BUF_LEN ETH_MAX_SIZE
     
 /** Macros **/
    #define msleep(ms) \
        usleep(ms*1000)
    #define debugf \
        if (dbg_debugOut) errorf
    #define printmac(f,v) \
        f("%02X:%02X:%02X:%02X:%02X:%02X", v[0], v[1], v[2], v[3], v[4], v[5]);

 /** Types **/
    #define bool unsigned char
    #define false 0
    #define true (!false)

#endif
