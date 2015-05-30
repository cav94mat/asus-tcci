/*
 * TrendChip Command Interpreter for ASUS-DSL routers
 * Version 0.1, by cav94mat.
 */
#include <stdio.h>
#include <stdlib.h>
//#include <errno.h>
#include <unistd.h>
//#include <signal.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <getopt.h>

#include <ra_ioctl.h>       /* @ /linux/linux-x.x.x.x/drivers/net/raeth.dsl */
#include "asus-tcci.h"

// Program status //
static bool tc_led = false;
static int exitReason = -1;
char* initcmd[TC_CMD_MAX_LEN];
char* tc_eth_adapter = DEF_ADAPTER;
char* tc_eth_remoteAdapter = DEF_ADAPTER_REMOTE;

// Debug options //
static int dbg_debugOut = false;
static int dbg_logPackets  = false;
static int dbg_blinkOnReceive = false;

// Ethernet options and status //    
static unsigned char tc_eth_remoteMac[6];
static unsigned char tc_eth_localMac[6];
static struct sockaddr_ll eth_SockAddr;
static unsigned int eth_adapterId;
static int tc_eth_sockIn;
static int tc_eth_sockOut = 0;

// From ra_reg_rw.c //
bool switch_rcv_okt(unsigned char* prcv_buf, unsigned short max_rcv_buf_size, unsigned short* prcv_buf_len)
{
    struct ifreq ifr;
    unsigned char ioctl_read_buf[MAX_TC_RESP_BUF_LEN+2];
    strncpy(ifr.ifr_name, tc_eth_remoteAdapter, strlen(tc_eth_remoteAdapter));
    ifr.ifr_data = ioctl_read_buf;
    if (-1 == ioctl(tc_eth_sockIn, RAETH_GET_TC_RESP, &ifr))
    {
        perror("-- ioctl");
        return false;
    }				
    unsigned short* pWord;
    pWord=(unsigned short*)(ioctl_read_buf);
    unsigned short pkt_len;
    pkt_len = *pWord++;
    *prcv_buf_len = 0;    
    if (pkt_len > 0 && max_rcv_buf_size >= pkt_len)
    {
        memcpy(prcv_buf, pWord,pkt_len);
        *prcv_buf_len = pkt_len;
    }
    return true;    							
}
// From tp_init.c //
/** printf to stderr */
void errorf(const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}
/** Get response as string (strip ethernet metadata) */
char* ethResponseToStr(unsigned char* x)
{
    if (*(x+14) != 0x07)
    {
        debugf("-- ethResponseToStr invalid or null!\n");
        return NULL;
    }
    return (char*)(x+15);
}
/** Get actual response (excluding ethernet metadata) length*/
int ethResponseLen(int len)
{
    if (len < 15)
    {
        debugf("-- ethResponseLen invalid: data is too short!\n");
        return 0;
    } else
        return (len-15);
}
/** Send ethernet frame */
bool sendPkt(unsigned char* SendBuf, unsigned short SendBufLen)
{
    eth_SockAddr.sll_family   = PF_PACKET;
    // we don't use a protocoll above ethernet layer just use anything here
    eth_SockAddr.sll_protocol = htons(ETH_P_IP);
    eth_SockAddr.sll_ifindex = eth_adapterId;
    // use any thing here
    eth_SockAddr.sll_hatype   = 0x1234;
    // target is another host
    eth_SockAddr.sll_pkttype  = PACKET_OTHERHOST;
    // address length
    eth_SockAddr.sll_halen    = ETH_ALEN;
    int send_result = 0;
    // send the packet
    send_result = sendto(tc_eth_sockOut, SendBuf, SendBufLen, 0,
                         (struct sockaddr*)&eth_SockAddr, sizeof(eth_SockAddr));
    return (send_result != -1);
}
/** Receive ethernet frame */
bool receivePkt(unsigned char* RcvBuf, unsigned short MaxRcvBufSize, unsigned short* pRcvBufLen)
{
    return (switch_rcv_okt(RcvBuf, MaxRcvBufSize, pRcvBufLen));
}
/** Record packet to log (if dbg_logPackets was activated) */
void logPkt(bool incoming, unsigned char* data , int size)
{
    int i,j,k=0;
    if (! dbg_logPackets)
        return;
    errorf("\r\n %s, length: %d\r\n", (incoming ? "IN" : "OUT"), size);
    //errorf("  Target MAC : %02X:%02X:%02X:%02X:%02X:%02X\n", data[k+=1],data[k+=1],data[k+=1],data[k+=1],data[k+=1],data[k+=1]);
    errorf("  Source MAC : ");
    printmac(errorf, (data+MAC_LEN));
    errorf("\n  Target MAC : ");
    printmac(errorf, data);
    k += (2 * MAC_LEN);
    //errorf("  Source MAC : %02X:%02X:%02X:%02X:%02X:%02X\n", data[k+=1],data[k+=1],data[k+=1],data[k+=1],data[k+=1],data[k+=1]);
    errorf("\n  Ether-Type : %02X %02X\n", data[k++],data[k++]);
    errorf("  Command    : %02X\n\n", data[k++]);
    errorf("  Data :\n");
    for(i= 0; i < size-k; i++)
    {
        if(i != 0 && i%16 == 0) // one line of hex printing is complete...
        {
            errorf("         ", i);
            for(j= i-16+k; j < i+k; j++)
            {
                if(data[j] >= 32 && data[j] <= 128)
                    errorf("%c",(unsigned char)data[j]); //if its a number or alphabet
                else
                    errorf("."); //otherwise print a dot
            }
            errorf("\n");
        }
        if(i%16 == 0)
            errorf("   ");
        errorf(" %02X",(unsigned int)data[i+k]);
        if(i == size-k-1)  //print the last spaces
        {
            for(j=0; j < 15-i%16; j++)
                errorf("   "); //extra spaces
            errorf("         ");
            for(j= i+k-i%16; j <= i+k; j++)
                if (data[j] >= 32 && data[j] <= 128)
                    errorf("%c", (unsigned char) data[j]);
                else
                    errorf(".");
            errorf("\n");
        }
    }
}
/** Initialize the specified adapter, and get the relative MAC address. */
void initAdapter(char* ifname)
{
    int fd;
    struct ifreq ifr;
    fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL)); /* open socket */
    strcpy(ifr.ifr_name, ifname);
    ioctl(fd, SIOCGIFHWADDR, &ifr); /* retrieve MAC address */
    close(fd);
    memcpy(tc_eth_localMac, ifr.ifr_hwaddr.sa_data, 6);
    debugf("Got MAC address of '%s': ", ifname);
    printmac(debugf, tc_eth_localMac);
    debugf("\n");
    // %02X:%02X:%02X:%02X:%02X:%02X\n", ifname, tc_eth_localMac[0],
    //    tc_eth_localMac[1], tc_eth_localMac[2],tc_eth_localMac[3],tc_eth_localMac[4],tc_eth_localMac[5]);
}
/** Send command to TrendChip CI */
bool tc_exec(char* cmd) {
    unsigned short i = 0;
    unsigned char pktBuf[ETH_MAX_SIZE];
    char command[TC_CMD_MAX_LEN];
    snprintf(command, TC_CMD_MAX_LEN, "%s%s", cmd, TC_LF);
    unsigned int commandLen = strlen(command);
    // Set dest MAC
    memcpy(&pktBuf[i], tc_eth_remoteMac, MAC_LEN);
    i+=MAC_LEN;
    // Set source MAC
    memcpy(&pktBuf[i], tc_eth_localMac, MAC_LEN);
    i+=MAC_LEN;
    // Set ETH-TYPE
    memcpy(&pktBuf[i], TC_ETH_TYPE, sizeof(TC_ETH_TYPE) - 1);
    i+=(sizeof(TC_ETH_TYPE) - 1);
    pktBuf[i] = TC_ETHF_REQUEST;
    i+=1;
    if (i+commandLen < sizeof(pktBuf))
    {
        debugf("-- Appending data (%d, %d)\n", i, commandLen);
        memcpy(&pktBuf[i], (unsigned char*)command, commandLen);
        i+=commandLen;
    }
    else
    {
        debugf("-- ETH frame overflow (command too long?)\n");
        return false;
    }
    debugf("-- Sending packet\n");
    logPkt(false, pktBuf, i); // Variable 'i' now holds the actual length of pktBuf.
    bool RetVal = sendPkt(pktBuf, i);
    if (!RetVal)
        errorf("-- E: Network error (write)!\n");
    return RetVal;
}

/** Receive responde from TrendChip CI (sync) */
bool tc_listen () {
    unsigned short RetRespLen;
    unsigned char  pktBuf[ETH_MAX_SIZE];
    if (receivePkt(pktBuf, ETH_MAX_SIZE, &RetRespLen))
    {
        if (RetRespLen == 0) // debugf("-- W: Got empty frame!\n"); 
        {
            if (dbg_blinkOnReceive)
                debugf((tc_led = !tc_led) ? "\r." : "\r ");
            msleep(100);
        }
        else
        {
            logPkt(true, pktBuf, RetRespLen);
            pktBuf[RetRespLen] = '\0';
            printf("%s", ethResponseToStr(pktBuf));
        }
    }
    else
    {
        debugf("-- Network error (read)\n");
        return false;
    }
    return true;
}
/** @main Application entry-point. */
int main(int argc, char* argv[])
{
    int c;
    strcpy(initcmd, DEF_CMD_INIT);
    while (true)
    {
        static struct option long_options[] =
        {
            {"adapter", required_argument, 0, 'a'},
            {"remote-adapter", required_argument, 0, 'b'},
            {"close", no_argument, 0, 'c'},
            {"blink-on-receive", no_argument, 0, 'k'},
            {"log-packets", no_argument, 0, 'p'},
            {"verbose", no_argument, 0, 'v'},
            {"version", no_argument, 0, 'V'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long (argc, argv, "a:b:ckpvV?", long_options, &option_index);
        
        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
            case 'a':
                tc_eth_adapter = optarg;
                break;
            case 'b':
                tc_eth_remoteAdapter = optarg;
                break;
            case 'c':
                exitReason = 0;
                break;
            case 'p':
                dbg_logPackets = true;
                break;
            case 'v':
                dbg_debugOut = true;
                break;
            case 'k':
                dbg_blinkOnReceive = true;
                break;
            case 'V':
                printf("Asus-TCCI (TrendChip Command Interpreter), version %s, build %s\n", VERSION, VERSION_BUILD);
                printf(" by %s. Sources and updates: %s\n", AUTHOR, URL);
                return EC_NORMAL;
            case 'h':
                /* getopt_long already printed an error message. */
                printf("usage: %s -V|--version\n", argv[0]); 
                printf("or: %s [-a|--adapter=\"%s\"] [-b|--remote-adapter=\"%s\"] [-c|--close] [-p|--log-packets] [-v|--verbose] [-k|--blink-on-receive] [<%s>]\n", argv[0], tc_eth_adapter, tc_eth_remoteAdapter, initcmd);
                
                return EC_NORMAL;
            default:
                return 9;
        }
    }
    
    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        initcmd[0] = '\0'; // Clear string
        while (optind < argc)
            strncat(initcmd, argv[optind++], sizeof(initcmd));
    }
    debugf("-- Starting...\n");
    // Initialize in-socket
    tc_eth_sockIn = socket(AF_INET, SOCK_DGRAM, 0);
    if (tc_eth_sockIn < 0)
    {
        errorf("-- E: Input socket init failed!\n");
        return EC_APIFAIL;
    }
	/*debugf("-- dbg_debugOut = %d, dbg_logPackets = %d, tc_eth_adapter = \"%s\", initcmd = \"%s\"\n",
	    dbg_debugOut, dbg_logPackets, tc_eth_adapter, initcmd);*/
	// Initialize adapter
	eth_adapterId = if_nametoindex(tc_eth_adapter);
    initAdapter(tc_eth_adapter);
    // Initialize out-socket
    tc_eth_sockOut = socket(AF_PACKET,SOCK_RAW, htons(ETH_P_ALL));
    if (tc_eth_sockOut < 0)
    {
        errorf("-- E: Output socket init failed!\n");
        return EC_APIFAIL;
    }
    // Acquired MACs
    debugf("Remote (TrendChip) MAC: ");
    printmac(debugf, tc_eth_remoteMac);
    debugf("\nLocal (terminal) MAC: ");
    printmac(debugf, tc_eth_localMac);
    // Initial command execution
    debugf("\n-- Phase 2 - Perform init-command (%s)\n", initcmd);
    if (!tc_exec(initcmd))
        return EC_NETFAIL;
    // Interactive shell
    char inbuff[TC_CMD_MAX_LEN];
    fd_set rfds;
    struct timeval tv;
    int retval;
    while(exitReason < 0) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; //100ms
        retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval == -1){
            perror("-- select(stdin)");
            return EC_APIFAIL;
        }
        else if (retval){   
            debugf("-- Gathering input... "); 
            char* cmd = fgets(inbuff, TC_CMD_MAX_LEN, stdin);
            if (cmd[strlen(cmd)-1] == '\n')
                cmd[strlen(cmd)-1] = '\0'; // Trim the trailing '\n' char.
            debugf("Got '%s'\n", cmd); 
            if (cmd[0] == '\0')
                {} // Do nothing on blank lines.
            else if (strcmp(cmd, "!q") == 0 || strcmp(cmd, "!quit") == 0)
                exitReason = EC_NORMAL;
            else if (strcmp(cmd, "!v") == 0 || strcmp(cmd, "!verbose") == 0)
                errorf("-- Verbose output is now %s.\n", (dbg_debugOut = !dbg_debugOut) ? "ON" : "OFF");
            else if (strcmp(cmd, "!p") == 0 || strcmp(cmd, "!logpackets") == 0)
                errorf("-- Packet logging is now %s.\n", (dbg_logPackets = !dbg_logPackets) ? "ON" : "OFF");
            else if (strcmp(cmd, "!k") == 0 || strcmp(cmd, "!blink") == 0)
                errorf("-- Blink-on-receive is now %s.\n", (dbg_blinkOnReceive = !dbg_blinkOnReceive) ? "ON" : "OFF");
            else if (strcmp(cmd, "!h") == 0 || strcmp(cmd, "!help") == 0)
                errorf("-- Valid client commands: \n--  ![blin]k, !h[elp], ![log]p[ackets], !q[uit], !v[erbose].\n"); 
            else if (cmd[0] == '!')
                errorf("-- Unrecognized client command. Please try \"!help\" if you aren't sure.\n"); 
            else if (!tc_exec(cmd))
                errorf("-- W: Command request fail!\n");
                //exitReason = EC_NETFAIL; // I/O error?
        }
        if (!tc_listen())
            exitReason = EC_NETFAIL;
    }
    if (tc_eth_sockIn) 
        close(tc_eth_sockIn);
    else
        debugf("-- W: Input socket already closed!");
    if (tc_eth_sockOut) 
        close(tc_eth_sockOut);
    else
        debugf("-- W: Output socket already closed!");
    debugf("-- Connection closed, reason = %d (-1).\n", exitReason);
    return exitReason-1;
}
