/* ESP8266 implementation of NetworkInterfaceAPI
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include "ESP8266Interface.h"
#include "mbed_debug.h"

// Various timeouts for different ESP8266 operations
#define ESP8266_CONNECT_TIMEOUT 15000
#define ESP8266_SEND_TIMEOUT    500
#define ESP8266_RECV_TIMEOUT    0
#define ESP8266_MISC_TIMEOUT    500

// Firmware version
#define ESP8266_VERSION 2

// ESP8266Interface implementation
ESP8266Interface::ESP8266Interface(PinName tx, PinName rx, bool debug)
    : _esp(tx, rx, debug)
{
    memset(_ids, 0, sizeof(_ids));
    memset(_cbs, 0, sizeof(_cbs));

    _esp.attach(this, &ESP8266Interface::event);
}

int ESP8266Interface::connect(const char *ssid, const char *pass, nsapi_security_t security,
                                        uint8_t channel)
{
    if (channel != 0) {
        return NSAPI_ERROR_UNSUPPORTED;
    }

    set_credentials(ssid, pass, security);
    return connect();
}

int ESP8266Interface::connect()
{
    _esp.setTimeout(ESP8266_CONNECT_TIMEOUT);
    
    if (!_esp.reset()) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }   
 
    _esp.setTimeout(ESP8266_MISC_TIMEOUT);
    
    if (_esp.get_firmware_version() != ESP8266_VERSION) {
        debug("ESP8266: ERROR: Firmware incompatible with this driver.\
               \r\nUpdate to v%d - https://developer.mbed.org/teams/ESP8266/wiki/Firmware-Update\r\n",ESP8266_VERSION); 
        return NSAPI_ERROR_DEVICE_ERROR;
    }
    
    _esp.setTimeout(ESP8266_CONNECT_TIMEOUT);

    if (!_esp.startup(3)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    if (!_esp.dhcp(true, 1)) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }

    if (!_esp.connect(_sta.ssid, _sta.pass)) {
        return NSAPI_ERROR_NO_CONNECTION;
    }

    if (!_esp.getIPAddress()) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }

    return NSAPI_ERROR_OK;
}

nsapi_error_t ESP8266Interface::gethostbyname(const char *name, SocketAddress *address, nsapi_version_t version)
{
    if (address->set_ip_address(name)) {
        if (version != NSAPI_UNSPEC && address->get_ip_version() != version) {
            return NSAPI_ERROR_DNS_FAILURE;
        }

        return NSAPI_ERROR_OK;
    }
    
    char *ipbuff = new char[NSAPI_IP_SIZE];
    int ret = 0;
    
    if(!_esp.dns_lookup(name, ipbuff)) {
        ret = NSAPI_ERROR_DEVICE_ERROR;
    } else {
        address->set_ip_address(ipbuff);
    }

    delete[] ipbuff;
    return ret;
}

int ESP8266Interface::start_soft_ap()
{
    _esp.setTimeout(ESP8266_CONNECT_TIMEOUT);

    if (!_esp.startup(3)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    if (!_esp.dhcp(true, 2)) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }

    if (!_esp.softAP(_ap.ssid, _ap.pass, _ap.sec, _ap.ch)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    if (!_esp.dhcps(_dhcps.start_ip, _dhcps.end_ip, _dhcps.lease_time, _dhcps.enable)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

int ESP8266Interface::stop_soft_ap()
{
    if (!_esp.dhcps(NULL, NULL, 0, false)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

int ESP8266Interface::start_soft_ap(const char *ssid, nsapi_security_t security, const char *pass,
        uint8_t channel, bool dhcps, const char *start_ip, const char *end_ip, int lease_time)

{
    set_ap_credentials(ssid, channel, security, pass);
    set_ap_dhcp_options(dhcps, start_ip, end_ip, lease_time);

    return start_soft_ap();
}

int ESP8266Interface::set_ap_credentials(const char *ssid, uint8_t channel, nsapi_security_t security,
        const char *pass)
{
    memset(_ap.ssid, 0, sizeof(_ap.ssid));
    /* XXX: This is wrong, SSID can be all NULL characters. Come back and fix.
     * Also, not guarantee that ssid is at least size of _ap.ssid
     */
    strncpy(_ap.ssid, ssid, sizeof(_ap.ssid));

    memset(_ap.pass, 0, sizeof(_ap.pass));
    strncpy(_ap.pass, pass, sizeof(_ap.pass));

    _ap.sec = security;
    _ap.ch = channel;

    return 0;
}

int ESP8266Interface::set_ap_options(bool dhcp, const char *ip, const char *netmask, const char *gateway)
{
    if (!_esp.ap_ip_options(dhcp, ip, netmask, gateway)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    memset(_ap.ip, 0, sizeof(_ap.ip));
    strncpy(_ap.ip, ip, sizeof(_ap.ip));

    memset(_ap.netmask, 0, sizeof(_ap.netmask));
    strncpy(_ap.netmask, netmask, sizeof(_ap.netmask));

    memset(_ap.gateway, 0, sizeof(_ap.gateway));
    strncpy(_ap.gateway, gateway, sizeof(_ap.gateway));

    return 0;
}

int ESP8266Interface::set_ap_dhcp_options(bool enable, const char *start_ip, const char *end_ip, int lease_time)
{
    if (enable == true &&
            ((start_ip == NULL || start_ip[0] == '\0') ||
            (end_ip == NULL || end_ip[0] == '\0')))
    {
        return -1;
    }

    _dhcps.enable = enable;

    memset(_dhcps.start_ip, 0, sizeof(_dhcps.start_ip));
    strncpy(_dhcps.start_ip, start_ip, sizeof(_dhcps.start_ip));

    memset(_dhcps.end_ip, 0, sizeof(_dhcps.end_ip));
    strncpy(_dhcps.end_ip, end_ip, sizeof(_dhcps.end_ip));

    _dhcps.lease_time = lease_time;

    return 0;
}

int ESP8266Interface::start_tcp_server(int port, int timeout)
{
    if (!_esp.tcpserver(true, port, timeout)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

int ESP8266Interface::stop_tcp_server()
{
    if (!_esp.tcpserver(false, 0, 0)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

int ESP8266Interface::set_credentials(const char *ssid, const char *pass, nsapi_security_t security)
{
    memset(_sta.ssid, 0, sizeof(_sta.ssid));
    strncpy(_sta.ssid, ssid, sizeof(_sta.ssid));

    memset(_sta.pass, 0, sizeof(_sta.pass));
    strncpy(_sta.pass, pass, sizeof(_sta.pass));

    _sta.sec = security;

    return 0;
}

int ESP8266Interface::set_channel(uint8_t channel)
{
    _sta.ch = channel;
    return NSAPI_ERROR_OK;
}


int ESP8266Interface::disconnect()
{
    _esp.setTimeout(ESP8266_MISC_TIMEOUT);

    if (!_esp.disconnect()) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

const char *ESP8266Interface::get_ip_address()
{
    return _esp.getIPAddress();
}

const char *ESP8266Interface::get_mac_address()
{
    return _esp.getMACAddress();
}

const char *ESP8266Interface::get_gateway()
{
    return _esp.getGateway();
}

const char *ESP8266Interface::get_netmask()
{
    return _esp.getNetmask();
}

int8_t ESP8266Interface::get_rssi()
{
    return _esp.getRSSI();
}

int ESP8266Interface::scan(WiFiAccessPoint *res, unsigned count)
{
    return _esp.scan(res, count);
}

struct esp8266_socket {
    int id;
    nsapi_protocol_t proto;
    bool connected;
    SocketAddress addr;
};

int ESP8266Interface::socket_open(void **handle, nsapi_protocol_t proto)
{
    // Look for an unused socket
    int id = -1;
 
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        if (!_ids[i]) {
            id = i;
            _ids[i] = true;
            break;
        }
    }
 
    if (id == -1) {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    struct esp8266_socket *socket = new struct esp8266_socket;
    if (!socket) {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    socket->id = id;
    socket->proto = proto;
    socket->connected = false;
    *handle = socket;
    return 0;
}

int ESP8266Interface::socket_close(void *handle)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    int err = 0;
    _esp.setTimeout(ESP8266_MISC_TIMEOUT);
 
    if (!_esp.close(socket->id)) {
        err = NSAPI_ERROR_DEVICE_ERROR;
    }

    _ids[socket->id] = false;
    delete socket;
    return err;
}

int ESP8266Interface::socket_bind(void *handle, const SocketAddress &address)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int ESP8266Interface::socket_listen(void *handle, int backlog)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int ESP8266Interface::socket_connect(void *handle, const SocketAddress &addr)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    _esp.setTimeout(ESP8266_MISC_TIMEOUT);

    const char *proto = (socket->proto == NSAPI_UDP) ? "UDP" : "TCP";
    if (!_esp.open(proto, socket->id, addr.get_ip_address(), addr.get_port())) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }
    
    socket->connected = true;
    return 0;
}
    
int ESP8266Interface::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int ESP8266Interface::socket_send(void *handle, const void *data, unsigned size)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    _esp.setTimeout(ESP8266_SEND_TIMEOUT);
 
    if (!_esp.send(socket->id, data, size)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }
 
    return size;
}

int ESP8266Interface::socket_recv(void *handle, void *data, unsigned size)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    _esp.setTimeout(ESP8266_RECV_TIMEOUT);
 
    int32_t recv = _esp.recv(socket->id, data, size);
    if (recv < 0) {
        return NSAPI_ERROR_WOULD_BLOCK;
    }
 
    return recv;
}

int ESP8266Interface::socket_sendto(void *handle, const SocketAddress &addr, const void *data, unsigned size)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;

    if (socket->connected && socket->addr != addr) {
        _esp.setTimeout(ESP8266_MISC_TIMEOUT);
        if (!_esp.close(socket->id)) {
            return NSAPI_ERROR_DEVICE_ERROR;
        }
        socket->connected = false;
    }

    if (!socket->connected) {
        int err = socket_connect(socket, addr);
        if (err < 0) {
            return err;
        }
        socket->addr = addr;
    }
    
    return socket_send(socket, data, size);
}

int ESP8266Interface::socket_recvfrom(void *handle, SocketAddress *addr, void *data, unsigned size)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;
    int ret = socket_recv(socket, data, size);
    if (ret >= 0 && addr) {
        *addr = socket->addr;
    }

    return ret;
}

void ESP8266Interface::socket_attach(void *handle, void (*callback)(void *), void *data)
{
    struct esp8266_socket *socket = (struct esp8266_socket *)handle;    
    _cbs[socket->id].callback = callback;
    _cbs[socket->id].data = data;
}

void ESP8266Interface::event() {
    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        if (_cbs[i].callback) {
            _cbs[i].callback(_cbs[i].data);
        }
    }
}
