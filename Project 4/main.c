/********************************************************
 * main.c
 *
 * SER486 – Project 4 Networking
 * Spring '26
 * Written By: Nathaniel Davis-Perez
 *
 * Full implementation of the main loop:
 *   – Initializes all hardware
 *   – Obtains a DHCP lease
 *   – Synchronises network time via NTP
 *   – Logs startup events and sends alarms
 *   – Then repeatedly services the watchdog, reads
 *     temperature, updates FSM, handles HTTP requests,
 *     and performs EEPROM write‑backs.
 ********************************************************/

#define HTTP_PORT       8080
#define SERVER_SOCKET   0

#define sei() __asm__ volatile ("sei" ::: "memory")

#include "config.h"
#include "delay.h"
#include "dhcp.h"
#include "led.h"
#include "log.h"
#include "rtc.h"
#include "spi.h"
#include "uart.h"
#include "vpd.h"
#include "temp.h"
#include "socket.h"
#include "alarm.h"
#include "wdt.h"
#include "tempfsm.h"
#include "eeprom.h"
#include "ntp.h"
#include "w51.h"
#include "signature.h"

/* Helper to build and send an HTTP JSON response */
static void send_http_response(void)
{
    /* Status line */
    socket_writestr(SERVER_SOCKET, "HTTP/1.1 200 OK\r\n");
    socket_writestr(SERVER_SOCKET, "Content-Type: application/vnd.api+json\r\n");
    socket_writestr(SERVER_SOCKET, "Connection: close\r\n");
    socket_writestr(SERVER_SOCKET, "\r\n");

    /* JSON payload */
    socket_writestr(SERVER_SOCKET, "{");
    /* VPD object */
    socket_writestr(SERVER_SOCKET, "\"vpd\":{");
    socket_writequotedstring(SERVER_SOCKET, "model"); socket_writestr(SERVER_SOCKET, ":");
    socket_writequotedstring(SERVER_SOCKET, vpd.model); socket_writestr(SERVER_SOCKET, ",");
    socket_writequotedstring(SERVER_SOCKET, "manufacturer"); socket_writestr(SERVER_SOCKET, ":");
    socket_writequotedstring(SERVER_SOCKET, vpd.manufacturer); socket_writestr(SERVER_SOCKET, ",");
    socket_writequotedstring(SERVER_SOCKET, "serial_number"); socket_writestr(SERVER_SOCKET, ":");
    socket_writequotedstring(SERVER_SOCKET, vpd.serial_number); socket_writestr(SERVER_SOCKET, ",");
    socket_writequotedstring(SERVER_SOCKET, "manufacture_date"); socket_writestr(SERVER_SOCKET, ":");
    socket_writedate(SERVER_SOCKET, vpd.manufacture_date); socket_writestr(SERVER_SOCKET, ",");
    socket_writequotedstring(SERVER_SOCKET, "mac_address"); socket_writestr(SERVER_SOCKET, ":");
    socket_write_macaddress(SERVER_SOCKET, vpd.mac_address); socket_writestr(SERVER_SOCKET, ",");
    socket_writequotedstring(SERVER_SOCKET, "country_code"); socket_writestr(SERVER_SOCKET, ":");
    socket_writequotedstring(SERVER_SOCKET, vpd.country_of_origin);
    socket_writestr(SERVER_SOCKET, "},");

    /* Config thresholds */
    socket_writestr(SERVER_SOCKET, "\"tcrit_hi\":"); socket_writedec32(SERVER_SOCKET, config.hi_alarm);
    socket_writestr(SERVER_SOCKET, ",\"twarn_hi\":"); socket_writedec32(SERVER_SOCKET, config.hi_warn);
    socket_writestr(SERVER_SOCKET, ",\"tcrit_lo\":"); socket_writedec32(SERVER_SOCKET, config.lo_alarm);
    socket_writestr(SERVER_SOCKET, ",\"twarn_lo\":"); socket_writedec32(SERVER_SOCKET, config.lo_warn);

    /* Current temperature and FSM state */
    socket_writestr(SERVER_SOCKET, ",\"temperature\":"); socket_writedec32(SERVER_SOCKET, temp_get());
    socket_writestr(SERVER_SOCKET, ",\"state\":"); socket_writequotedstring(SERVER_SOCKET, tempfsm_get_state());

    /* Event log */
    socket_writestr(SERVER_SOCKET, ",\"log\":[");
    unsigned char entries = log_get_num_entries();
    for (unsigned char i = 0; i < entries; i++) {
        unsigned long ts;
        unsigned char ev;
        if (log_get_record(i, &ts, &ev)) {
            socket_writestr(SERVER_SOCKET, "{\"timestamp\":");
            socket_writequotedstring(SERVER_SOCKET, rtc_num2datestr(ts));
            socket_writestr(SERVER_SOCKET, ",\"event\":");
            socket_writedec32(SERVER_SOCKET, ev);
            socket_writestr(SERVER_SOCKET, "}");
            if (i < entries - 1) socket_writestr(SERVER_SOCKET, ",");
        }
    }
    socket_writestr(SERVER_SOCKET, "]}");
    socket_writestr(SERVER_SOCKET, "\r\n\r\n");

    /* Disconnect after response (Connection: close) */
    socket_disconnect(SERVER_SOCKET);
}

int main(void)
{
    sei();   /* enable global interrupts */

    /* 1. Initialize hardware and subsystems */
    uart_init();
    uart_writestr("\r\n--- SER486 Project 4 IoT Device ---\r\n");
    led_init();
    vpd_init();
    config_init();
    log_init();
    rtc_init();
    spi_init();
    temp_init();
    W5x_init();
    tempfsm_init();

    /* Sign the assignment */
    signature_set("Nathaniel","Davis-Perez","ndavispe");

    /* 2. Obtain DHCP lease */
    unsigned char blank_addr[] = {0,0,0,0};
    W5x_config(vpd.mac_address, blank_addr, blank_addr, blank_addr);
    uart_writestr("Starting DHCP...\r\n");
    while (!dhcp_start(vpd.mac_address, 60000UL, 4000UL)) {}
    uart_writestr("local ip: "); uart_writeip(dhcp_getLocalIp()); uart_writestr("\r\n");

    W5x_config(vpd.mac_address, dhcp_getLocalIp(), dhcp_getGatewayIp(), dhcp_getSubnetMask());

    /* 3. Synchronise with NTP */
    log_add_record(EVENT_TIMESET);
    ntp_sync_network_time(5);
    log_add_record(EVENT_NEWTIME);

    /* 4. Start watchdog */
    wdt_init();

    /* Log startup and send alarm */
    log_add_record(EVENT_STARTUP);
    alarm_send(EVENT_STARTUP);

    /* Check for test start request */
    check_for_test_start();

    /* 5. Main loop */
    while (1) {
        wdt_reset();

        /* Update LED blink state machine */
        led_update();

        /* Temperature sensor processing every 1 second */
        if (delay_isdone(1)) {
            int cur_temp = temp_get();
            tempfsm_update(cur_temp, config.hi_alarm, config.hi_warn,
                           config.lo_alarm, config.lo_warn);
            delay_set(1, 1000);   /* 1 second periodic read */
        }

        /* TCP socket management */
        if (socket_is_closed(SERVER_SOCKET)) {
            socket_open(SERVER_SOCKET, HTTP_PORT);
            socket_listen(SERVER_SOCKET);
        }

        if (socket_recv_available(SERVER_SOCKET)) {
            /* parse HTTP request (simplified; in real code parse method/URI) */
            char method[8];
            if (socket_recv_compare(SERVER_SOCKET, "GET ")) {
                /* read rest of line and headers */
                socket_flush_line(SERVER_SOCKET);   /* flush request URI */
                while (socket_received_line(SERVER_SOCKET) && !socket_is_blank_line(SERVER_SOCKET))
                    socket_flush_line(SERVER_SOCKET);
                if (socket_is_blank_line(SERVER_SOCKET)) socket_flush_line(SERVER_SOCKET);
                send_http_response();
            } else {
                /* unsupported method – close */
                socket_writestr(SERVER_SOCKET, "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n");
                socket_disconnect(SERVER_SOCKET);
            }
        }

        /* EEPROM write‑backs */
        log_update();
        config_update();
    }
    return 0;
}