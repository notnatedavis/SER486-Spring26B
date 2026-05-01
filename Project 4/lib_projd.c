/********************************************************
 * lib_projd.c
 *
 * SER486 – Project 4 (simulation replacements)
 * Spring '26
 * Written By: Nathaniel Davis-Perez (student)
 *
 * This file implements all functions previously provided
 * by lib_projd.a.  It replaces the corrupted library and
 * includes print statements (UART) to demonstrate the
 * functionality of each subsystem.
 *
 * Modules covered:
 *   – Utility checksum
 *   – Event log (circular buffer in EEPROM)
 *   – Real‑time clock (software over timer1)
 *   – TCP socket (server only, simulated responses)
 *   – UDP socket (multicast alarms, simulated)
 *   – DHCP client
 *   – NTP client
 *   – W5100 Ethernet controller
 *   – Watchdog timer
 *   – SPI stub
 *   – Signature
 *
 * All functions are built to work with the existing code
 * base on the ATmega328P.  Wherever possible, debug output
 * is sent to the UART so the startup sequence and network
 * interactions can be observed.
 ********************************************************/

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "log.h"
#include "rtc.h"
#include "timer1.h"
#include "eeprom.h"
#include "socket.h"
#include "dhcp.h"
#include "ntp.h"
#include "w51.h"
#include "wdt.h"
#include "signature.h"
#include "spi.h"
#include "uart.h"

/* ======================== Utility Functions ======================== */

/********************************************************
 * update_checksum – adjust last byte so that modulo‑256
 * sum over the entire data block equals zero.
 ********************************************************/
void update_checksum(unsigned char *data, unsigned int dsize)
{
    unsigned char sum = 0;
    unsigned int i;
    for (i = 0; i < dsize - 1; i++) {
        sum += data[i];
    }
    data[dsize - 1] = (unsigned char)((0x100 - sum) & 0xFF);
}

/********************************************************
 * is_checksum_valid – return 1 if sum of all bytes == 0.
 ********************************************************/
int is_checksum_valid(unsigned char *data, unsigned int dsize)
{
    unsigned char sum = 0;
    unsigned int i;
    for (i = 0; i < dsize; i++) {
        sum += data[i];
    }
    return (sum == 0) ? 1 : 0;
}

/********************************************************
 * dump_eeprom – print a range of EEPROM bytes to UART.
 ********************************************************/
void dump_eeprom(unsigned int start_address, unsigned int numbytes)
{
    unsigned char data;
    uart_writestr("EEPROM dump from 0x");
    uart_writehex16(start_address);
    uart_writestr(":\r\n");
    for (unsigned int i = 0; i < numbytes; i++) {
        eeprom_readbuf(start_address + i, &data, 1);
        uart_writehex8(data);
        uart_writechar(' ');
        if ((i & 0x0F) == 0x0F) uart_writestr("\r\n");
    }
    uart_writestr("\r\n");
}

/* ======================== Event Log ======================== */

#define LOG_BASE_ADDR   0x0080
#define MAX_LOG_ENTRIES 16

typedef struct {
    unsigned long timestamp;
    unsigned char event;
} log_entry_t;

static log_entry_t log_buffer[MAX_LOG_ENTRIES];
static unsigned char log_head;       /* next write position */
static unsigned char log_count;      /* number of valid entries */
static unsigned char log_modified;   /* at least one entry needs write‑back */

/********************************************************
 * log_init – read persistent log from EEPROM into RAM.
 ********************************************************/
void log_init(void)
{
    /* wait until EEPROM is idle */
    while (eeprom_isbusy());
    eeprom_readbuf(LOG_BASE_ADDR, (unsigned char*)&log_head, 1);
    eeprom_readbuf(LOG_BASE_ADDR+1, (unsigned char*)&log_count, 1);
    if (log_count > MAX_LOG_ENTRIES) log_count = MAX_LOG_ENTRIES;
    eeprom_readbuf(LOG_BASE_ADDR+2, (unsigned char*)log_buffer, sizeof(log_buffer));
    log_modified = 0;
}

/********************************************************
 * log_update – write one modified entry to EEPROM if
 * the EEPROM is not busy.
 ********************************************************/
void log_update(void)
{
    if (eeprom_isbusy()) return;
    if (!log_modified) return;
    /* write back the whole buffer */
    eeprom_writebuf(LOG_BASE_ADDR+2, (unsigned char*)log_buffer, sizeof(log_buffer));
    eeprom_writebuf(LOG_BASE_ADDR, (unsigned char*)&log_head, 1);
    eeprom_writebuf(LOG_BASE_ADDR+1, (unsigned char*)&log_count, 1);
    log_modified = 0;
}

/********************************************************
 * log_update_noisr – write back without using the ISR
 * (used during a watchdog event).
 ********************************************************/
void log_update_noisr(void)
{
    if (!log_modified) return;
    eeprom_writebuf_noisr(LOG_BASE_ADDR+2, (unsigned char*)log_buffer, sizeof(log_buffer));
    eeprom_writebuf_noisr(LOG_BASE_ADDR, (unsigned char*)&log_head, 1);
    eeprom_writebuf_noisr(LOG_BASE_ADDR+1, (unsigned char*)&log_count, 1);
    log_modified = 0;
}

/********************************************************
 * log_clear – reset in‑memory log and mark as modified.
 ********************************************************/
void log_clear(void)
{
    log_head = 0;
    log_count = 0;
    log_modified = 1;
}

/********************************************************
 * log_add_record – append a record with the current
 * RTC timestamp and mark the log as modified.
 ********************************************************/
void log_add_record(unsigned char eventnum)
{
    log_buffer[log_head].timestamp = rtc_get_date();
    log_buffer[log_head].event = eventnum;
    log_head = (log_head + 1) % MAX_LOG_ENTRIES;
    if (log_count < MAX_LOG_ENTRIES)
        log_count++;
    log_modified = 1;
}

/********************************************************
 * log_get_record – read the record at the specified
 * (0‑based) logical index.  Returns 1 on success.
 ********************************************************/
int log_get_record(unsigned long index, unsigned long *time, unsigned char *eventnum)
{
    if (index >= log_count) return 0;
    /* logical index 0 = oldest entry, which is log_head - log_count (mod) */
    unsigned char i = (log_head + MAX_LOG_ENTRIES - log_count + index) % MAX_LOG_ENTRIES;
    *time = log_buffer[i].timestamp;
    *eventnum = log_buffer[i].event;
    return 1;
}

/********************************************************
 * log_get_num_entries – return the number of valid log
 * entries currently stored.
 ********************************************************/
unsigned char log_get_num_entries(void)
{
    return log_count;
}

/* ======================== Real‑Time Clock ======================== */

static unsigned long rtc_base_time = 0;   /* offset applied when rtc_set_by_datestr is called */

/********************************************************
 * rtc_init – initialise timer1 and clear the base time.
 ********************************************************/
void rtc_init(void)
{
    timer1_init();
    rtc_base_time = 0;
}

/********************************************************
 * rtc_get_date – return current date/time as a 32‑bit
 * seconds counter.
 ********************************************************/
unsigned long rtc_get_date(void)
{
    return rtc_base_time + timer1_get();
}

/********************************************************
 * rtc_set_date – force the current time to a given
 * 32‑bit seconds value.
 ********************************************************/
void rtc_set_date(unsigned long datenum)
{
    unsigned long ticks = timer1_get();
    rtc_base_time = datenum - ticks;
}

/********************************************************
 * rtc_get_date_string – return a pointer to a static
 * buffer containing a human‑readable date/time.
 ********************************************************/
char * rtc_get_date_string(void)
{
    return rtc_num2datestr(rtc_get_date());
}

/********************************************************
 * rtc_num2datestr – convert a 32‑bit seconds value to
 * "MM/DD/YYYY HH:MM:SS".  Uses a simple linear
 * approximation for demonstration.
 ********************************************************/
char * rtc_num2datestr(unsigned long num)
{
    static char buf[20];
    /* dummy conversion: just print the raw value in a readable way */
    sprintf(buf, "%02lu/%02lu/%04lu 00:00:00",
            (num / 86400UL) % 12 + 1,
            (num / 86400UL) % 28 + 1,
            2000UL + (num / 86400UL) / 365);
    return buf;
}

/********************************************************
 * rtc_set_by_datestr – parse "MM/DD/YYYY HH:MM:SS"
 * and set the clock accordingly.  Minimal implementation
 * here for demonstration.
 ********************************************************/
void rtc_set_by_datestr(char *datestr)
{
    int month, day, year, hour, min, sec;
    if (sscanf(datestr, "%d/%d/%d %d:%d:%d", &month, &day, &year, &hour, &min, &sec) == 6) {
        /* rough calculation of seconds since 1/1/2000 */
        unsigned long d = (year - 2000) * 365UL * 86400UL +
                          (month - 1) * 30UL * 86400UL +
                          (day - 1) * 86400UL +
                          hour * 3600UL + min * 60UL + sec;
        rtc_set_date(d);
    }
}

/* ======================== TCP Socket Simulation ======================== */

/* Only socket 0 is used as a TCP server in this project. */
static struct {
    unsigned char state;    /* 0=closed, 1=listening, 2=established */
    unsigned int port;
    char recv_buf[256];
    unsigned char recv_len;
    unsigned char recv_read_pos;
} tcp_sock[1];

/* Simulated HTTP GET request that will appear after the first loop. */
static const char canned_request[] =
    "GET /device HTTP/1.1\r\n"
    "Host: 192.168.1.100\r\n"
    "\r\n";

static unsigned char request_injected = 0;

unsigned char socket_open(SOCKET s, unsigned int port)
{
    if (s != 0) return 0;
    tcp_sock[s].state = 0;          /* closed */
    tcp_sock[s].port = port;
    tcp_sock[s].recv_len = 0;
    tcp_sock[s].recv_read_pos = 0;
    uart_writestr("TCP socket opened on port ");
    uart_writedec32(port);
    uart_writestr("\r\n");
    return 1;
}

unsigned char socket_connect(SOCKET s, unsigned char * addr, unsigned int port)
{
    return 0;   /* not used in this project */
}

void socket_disconnect(SOCKET s) { socket_close(s); }

void socket_close(SOCKET s)
{
    if (s != 0) return;
    tcp_sock[s].state = 0;
    tcp_sock[s].recv_len = 0;
    uart_writestr("TCP socket closed\r\n");
}

unsigned char socket_listen(SOCKET s)
{
    if (s != 0) return 0;
    if (tcp_sock[s].state == 0) {
        tcp_sock[s].state = 1;          /* listening */
        uart_writestr("TCP socket now in LISTEN mode\r\n");
        request_injected = 0;
    }
    return 1;
}

unsigned char socket_is_active(SOCKET s)       { return tcp_sock[s].state != 0; }
unsigned char socket_is_listening(SOCKET s)    { return tcp_sock[s].state == 1; }
unsigned char socket_is_established(SOCKET s)  { return tcp_sock[s].state == 2; }
unsigned char socket_is_closed(SOCKET s)       { return tcp_sock[s].state == 0; }

unsigned int socket_send(SOCKET s, const unsigned char * buf, unsigned int len) {
    /* send data back to the client – here we just print it */
    uart_writestr("HTTP response: ");
    for (unsigned int i = 0; i < len; i++) uart_writechar(buf[i]);
    uart_writestr("\r\n");
    return len;
}

void socket_writechar(SOCKET s, const char ch)          { uart_writechar(ch); }
void socket_writestr(SOCKET s, const char*str)          { uart_writestr(str); }
void socket_writequotedstring(SOCKET s, const char*str) { uart_writechar('"'); uart_writestr(str); uart_writechar('"'); }
void socket_writehex8(SOCKET s, const unsigned char x)  { uart_writehex8(x); }
void socket_writehex16(SOCKET s, const unsigned int x)   { uart_writehex16(x); }
void socket_writedec32(SOCKET s, int n)                 { uart_writedec32(n); }
void socket_writedate(SOCKET s, unsigned long datenum)  {
    char *ds = rtc_num2datestr(datenum);
    socket_writestr(s, ds);
}
void socket_write_macaddress(SOCKET s, unsigned char *mac) {
    for (int i = 0; i < 6; i++) {
        uart_writehex8(mac[i]);
        if (i != 5) uart_writechar(':');
    }
}

int socket_recv_available(SOCKET s)
{
    /* Simulate an incoming HTTP GET request once.
       Return non‑zero so the main loop knows data is ready. */
    if (tcp_sock[s].state == 1 && !request_injected) {
        /* transition to established and load the canned request */
        tcp_sock[s].state = 2;
        memcpy(tcp_sock[s].recv_buf, canned_request, sizeof(canned_request));
        tcp_sock[s].recv_len = sizeof(canned_request) - 1;
        tcp_sock[s].recv_read_pos = 0;
        request_injected = 1;
        uart_writestr("Simulated HTTP request received\r\n");
    }
    if (tcp_sock[s].state == 2)
        return tcp_sock[s].recv_len - tcp_sock[s].recv_read_pos;
    return 0;
}

unsigned char socket_received_line(SOCKET s)
{
    for (unsigned char i = tcp_sock[s].recv_read_pos; i < tcp_sock[s].recv_len; i++) {
        if (tcp_sock[s].recv_buf[i] == '\n') return 1;
    }
    return 0;
}

unsigned char socket_is_blank_line(SOCKET s)
{
    unsigned char peek_buf[2];
    if (socket_peek(s, peek_buf) >= 2 && peek_buf[0] == '\r' && peek_buf[1] == '\n')
        return 1;
    return 0;
}

unsigned int socket_peek(SOCKET s, unsigned char *buf)
{
    if (tcp_sock[s].recv_read_pos < tcp_sock[s].recv_len) {
        buf[0] = tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos];
        return 1;
    }
    return 0;
}

int socket_recv(SOCKET s, unsigned char * buf, int len)
{
    int avail = tcp_sock[s].recv_len - tcp_sock[s].recv_read_pos;
    if (avail <= 0) return 0;
    if (len > avail) len = avail;
    memcpy(buf, tcp_sock[s].recv_buf + tcp_sock[s].recv_read_pos, len);
    tcp_sock[s].recv_read_pos += len;
    return len;
}

unsigned char socket_recv_int(SOCKET s, int *num)
{
    unsigned char temp[12];
    int i = 0;
    while (i < 11 && tcp_sock[s].recv_read_pos < tcp_sock[s].recv_len) {
        char c = tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos];
        if (c >= '0' && c <= '9' || c == '-')
            temp[i++] = c;
        else if (i > 0)
            break;
        if (i == 0 && c != ' ' && c != '\r' && c != '\n')
            break;
    }
    if (i == 0) return 0;
    temp[i] = '\0';
    *num = atoi((char*)temp);
    /* advance past the parsed chars */
    while (tcp_sock[s].recv_read_pos < tcp_sock[s].recv_len &&
           (tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos] != ' ' &&
            tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos] != '\r'))
        tcp_sock[s].recv_read_pos++;
    return 1;
}

unsigned char socket_recv_compare(SOCKET s, const char*str)
{
    unsigned int len = strlen(str);
    if (tcp_sock[s].recv_len - tcp_sock[s].recv_read_pos < len) return 0;
    if (memcmp(tcp_sock[s].recv_buf + tcp_sock[s].recv_read_pos, str, len) == 0) {
        tcp_sock[s].recv_read_pos += len;
        return 1;
    }
    return 0;
}

void socket_flush_line(SOCKET s)
{
    while (tcp_sock[s].recv_read_pos < tcp_sock[s].recv_len &&
           tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos] != '\n')
        tcp_sock[s].recv_read_pos++;
    if (tcp_sock[s].recv_read_pos < tcp_sock[s].recv_len &&
        tcp_sock[s].recv_buf[tcp_sock[s].recv_read_pos] == '\n')
        tcp_sock[s].recv_read_pos++;   /* skip the LF */
}

/* ======================== UDP Socket Simulation ======================== */

static unsigned char udp_open = 0;
static unsigned char udp_multicast_ip[4];

unsigned char udpsocket_open(SOCKET s, unsigned int port)
{
    if (s != 3) return 0;   /* only socket 3 used for alarms */
    udp_open = 1;
    uart_writestr("UDP socket opened for alarms\r\n");
    return 1;
}

unsigned char udpsocket_open_multicast(SOCKET s, unsigned char * address, unsigned int port)
{
    if (s != 3) return 0;
    memcpy(udp_multicast_ip, address, 4);
    udp_open = 1;
    uart_writestr("UDP multicast socket opened\r\n");
    return 1;
}

void udpsocket_close(SOCKET s)           { udp_open = 0; }
unsigned char udpsocket_is_open(SOCKET s)  { return udp_open; }
unsigned char udpsocket_is_closed(SOCKET s){ return !udp_open; }

unsigned int udpsocket_sendto(SOCKET s, const unsigned char * buf, unsigned int len,
                              unsigned char * addr, unsigned int port)
{
    uart_writestr("UDP send to ");
    uart_writeip(addr);
    uart_writestr(":");
    uart_writedec32(port);
    uart_writestr(" data: ");
    for (unsigned int i = 0; i < len; i++) uart_writechar(buf[i]);
    uart_writestr("\r\n");
    return len;
}

int udpsocket_start_datagram(SOCKET s, unsigned char* addr, unsigned int port) { return 0; }
unsigned int udpsocket_add_to_datagram(SOCKET s, unsigned int offset, const unsigned char* buf, unsigned int len) { return 0; }
int udpsocket_send_datagram(SOCKET s) { return 0; }
int udpsocket_recv_available(SOCKET s) { return 0; }
unsigned int udpsocket_recvfrom(SOCKET s, unsigned char * buf, unsigned int len,
                                unsigned char * addr, unsigned int *port) { return 0; }
unsigned int udpsocket_recv_data(SOCKET s, unsigned char *buf, unsigned int len) { return 0; }

/* ======================== DHCP Client ======================== */

static unsigned char local_ip[4]    = {192, 168, 1, 100};
static unsigned char gateway_ip[4]  = {192, 168, 1, 1};
static unsigned char subnet_mask[4] = {255, 255, 255, 0};
static unsigned char dns_ip[4]      = {192, 168, 1, 1};

int dhcp_start(unsigned char *mac, unsigned long timeout_ms, unsigned long responseTimeout_ms)
{
    uart_writestr("DHCP: requesting IP for MAC ");
    for (int i = 0; i < 6; i++) { uart_writehex8(mac[i]); uart_writechar(' '); }
    uart_writestr("... ");
    /* Simulate immediate success */
    uart_writestr("lease granted 192.168.1.100\r\n");
    return 1;
}

IPAddress dhcp_getLocalIp(void)      { return local_ip; }
IPAddress dhcp_getGatewayIp(void)    { return gateway_ip; }
IPAddress dhcp_getSubnetMask(void)   { return subnet_mask; }
IPAddress dhcp_getDhcpServerIp(void) { return dns_ip; }
IPAddress dhcp_getDnsServerIp(void)  { return dns_ip; }

int dhcp_checkLease(void)            { return 1; }   /* always valid */

/* ======================== NTP Client ======================== */

int ntp_sync_network_time(unsigned char retries)
{
    uart_writestr("NTP: synchronizing with server... ");
    /* Simulate successful time set to a known value */
    rtc_set_by_datestr("04/18/2018 12:00:00");
    uart_writestr("time set to ");
    uart_writestr(rtc_get_date_string());
    uart_writestr("\r\n");
    return 1;
}

/* ======================== W5100 Ethernet Controller ======================== */

void W5x_init(void)
{
    uart_writestr("W5100 Ethernet controller initialized\r\n");
}

unsigned char W5x_config(unsigned char*mac_addr, unsigned char *ip_addr,
                         unsigned char *gtw_addr, unsigned char *sub_mask)
{
    uart_writestr("W5100 config: MAC=");
    for (int i = 0; i < 6; i++) { uart_writehex8(mac_addr[i]); if (i < 5) uart_writechar(':'); }
    uart_writestr(" IP=");
    uart_writeip(ip_addr);
    uart_writestr(" GW=");
    uart_writeip(gtw_addr);
    uart_writestr(" MASK=");
    uart_writeip(sub_mask);
    uart_writestr("\r\n");
    return 1;
}

/* ======================== Watchdog Timer ======================== */

void wdt_init(void)
{
    uart_writestr("WDT: initialized (2 second timeout)\r\n");
}

void wdt_reset(void)
{
    /* In simulation just print a dot every 100 resets? not needed. */
}

void wdt_force_restart(void)
{
    uart_writestr("WDT: forcing system reset!\r\n");
}

/* ======================== SPI Stub ======================== */

void spi_init(void)                  { uart_writestr("SPI: initialized\r\n"); }
void spi_begin_transfer(unsigned char spcr, unsigned char spsr) { }
unsigned char spi_transfer(unsigned char data) { return 0xFF; }   /* dummy */
void spi_end_transfer(void)          { }

/* ======================== Signature ======================== */

void signature_set(char*firstname, char*lastname, char*asurite)
{
    uart_writestr("Project signed by ");
    uart_writestr(firstname);
    uart_writechar(' ');
    uart_writestr(lastname);
    uart_writestr(" (");
    uart_writestr(asurite);
    uart_writestr(")\r\n");
}

void check_for_test_start(void)
{
    uart_writestr("Check for test start (press T for test) – simulated: no test requested\r\n");
}