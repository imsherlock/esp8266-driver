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

#ifndef ESP8266_INTERFACE_H
#define ESP8266_INTERFACE_H

#include "mbed.h"
#include "ESP8266.h"


#define ESP8266_SOCKET_COUNT 5

/** ESP8266Interface class
 *  Implementation of the NetworkStack for the ESP8266
 */
class ESP8266Interface : public NetworkStack, public WiFiInterface
{
public:
    /** ESP8266Interface lifetime
     * @param tx        TX pin
     * @param rx        RX pin
     * @param debug     Enable debugging
     */
    ESP8266Interface(PinName tx, PinName rx, bool debug = false);

    /** Start the interface
     *
     *  Attempts to connect to a WiFi network. Requires ssid and passphrase to be set.
     *  If passphrase is invalid, NSAPI_ERROR_AUTH_ERROR is returned.
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual int connect();

    /** Start the interface
     *
     *  Attempts to connect to a WiFi network.
     *
     *  @param ssid      Name of the network to connect to
     *  @param pass      Security passphrase to connect to the network
     *  @param security  Type of encryption for connection (Default: NSAPI_SECURITY_NONE)
     *  @param channel   This parameter is not supported, setting it to anything else than 0 will result in NSAPI_ERROR_UNSUPPORTED
     *  @return          0 on success, or error code on failure
     */
    virtual int connect(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE,
                                  uint8_t channel = 0);
    
    /** Translates a hostname to an IP address with specific version
     *
     *  The hostname may be either a domain name or an IP address. If the
     *  hostname is an IP address, no network transactions will be performed.
     *
     *
     *  @param host     Hostname to resolve
     *  @param address  Destination for the host SocketAddress
     *  @param version  IP version of address to resolve, NSAPI_UNSPEC indicates
     *                  version is chosen by the stack (defaults to NSAPI_UNSPEC)
     *  @return         0 on success, negative error code on failure
     */
    virtual nsapi_error_t gethostbyname(const char *name, SocketAddress *address, nsapi_version_t version);

    /**
     * Starts the Soft AP mode.
     *
     * @return Returns NSAPI_ERROR_OK on success, error otherwise.
     */
    int start_soft_ap();

    /**
     * Stops the Soft AP mode from running.
     *
     * @return Returns NSAPI_ERROR_OK on success, error otherwise.
     */
    int stop_soft_ap();

    /**
     * Starts the Soft AP mode.
     *
     * @param ssid The name of the AP SSID to use.
     * @param security (optional) The type of security to use for the connection (default is NONE).
     * @param pass (optional) The password to use for secure connections (If security is enabled password is minimum of 8 characters).
     * @param channel (optional) The channel to use for WiFi connections (default is 0).
     * @param dhcps (optional) Enable the DHCP server for the AP (default is disabled).
     * @param start_ip (optional) String representation of the starting IPv4 address for DHCP (must be provided if DHCP is enabled).
     * @param end_ip (optional) String representation of the ending IPv4 address for DHCP (must be provided if DHCP is enabled).
     * @param lease_time (optional) DHCP lease time for clients when server is enabled (default is 2880 minutes).
     * @return Returns NSAPI_ERROR_OK on success, failure otherwise.
     */
    int start_soft_ap(const char *ssid, nsapi_security_t security = NSAPI_SECURITY_NONE, const char *pass = NULL,
            uint8_t channel = 0, bool dhcps = false, const char *start_ip = NULL, const char *end_ip = NULL,
            int lease_time = 2880);

    /**
     * Set the AP credentials for wireless clients to connect.
     *
     * @param ssid The name of the network to connect to.
     * @param channel (optional) The channel to use (default is 0).
     * @param security (optional) The type of security to use for connections (default is NONE).
     * @param pass (optional) The password to use for wireless security (if security is set then password is minimum of 8 characters).
     * @return Returns 0 on success, -errno otherwise.
     */
    int set_ap_credentials(const char *ssid, uint8_t channel = 0, nsapi_security_t security = NSAPI_SECURITY_NONE,
            const char *pass = NULL);

    /**
     * Sets the IP options for soft AP.
     *
     * @param dhcp Enable or disable DHCP.
     * @param ip (optional) The IPv4 address if DHCP is disabled.
     * @param netmask (optional) The IPv4 netmask of the network if DHCP is disabled.
     * @param gateway (optional) The IPV4 gateway of the network if DHCP is disabled.
     * @return Returns 0 on success, -errno otherwise.
     */
    int set_ap_options(bool dhcp, const char *ip = NULL, const char *netmask = NULL, const char *gateway = NULL);

    /**
     * Set the AP options for DHCP server.
     *
     * @param enable Enable or disable the DHCP server for the AP.
     * @param start_ip (optional) String representation of the starting IPv4 address for leases.
     * @param end_ip (optional) String representation of the end IPv4 address for leases.
     * @param lease_time (optional) The lease time for DHCP clients (default is 2880 minutes).
     * @return Returns 0 on success, -errno otherwise.
     */
    int set_ap_dhcp_options(bool enable, const char *start_ip = NULL, const char *end_ip = NULL, int lease_time = 2880);

    /**
     * Starts the TCP server.
     *
     * @param port (optional) The port number to listen on for incoming TCP connections (default 80).
     * @param timeout (optional) The timeout for closing incoming connections (default 1440).
     * @return Returns NSAPI_ERROR_OK on success, error otherwise.
     */
    int start_tcp_server(int port = 80, int timeout = 1440);

    /**
     * Stops the TCP server.
     *
     * @return Returns NSAPI_ERROR_OK on success, error otherwise.
     */
    int stop_tcp_server();

    /** Set the WiFi network credentials
     *
     *  @param ssid      Name of the network to connect to
     *  @param pass      Security passphrase to connect to the network
     *  @param security  Type of encryption for connection
     *                   (defaults to NSAPI_SECURITY_NONE)
     *  @return          0 on success, or error code on failure
     */
    virtual int set_credentials(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE);

    /** Set the WiFi network channel - NOT SUPPORTED
     *
     * This function is not supported and will return NSAPI_ERROR_UNSUPPORTED
     *
     *  @param channel   Channel on which the connection is to be made, or 0 for any (Default: 0)
     *  @return          Not supported, returns NSAPI_ERROR_UNSUPPORTED
     */
    virtual int set_channel(uint8_t channel);

    /** Stop the interface
     *  @return             0 on success, negative on failure
     */
    virtual int disconnect();

    /** Get the internally stored IP address
     *  @return             IP address of the interface or null if not yet connected
     */
    virtual const char *get_ip_address();

    /** Get the internally stored MAC address
     *  @return             MAC address of the interface
     */
    virtual const char *get_mac_address();

     /** Get the local gateway
     *
     *  @return         Null-terminated representation of the local gateway
     *                  or null if no network mask has been recieved
     */
    virtual const char *get_gateway();

    /** Get the local network mask
     *
     *  @return         Null-terminated representation of the local network mask
     *                  or null if no network mask has been recieved
     */
    virtual const char *get_netmask();

    /** Gets the current radio signal strength for active connection
     *
     * @return          Connection strength in dBm (negative value)
     */
    virtual int8_t get_rssi();

    /** Scan for available networks
     *
     * This function will block.
     *
     * @param  ap       Pointer to allocated array to store discovered AP
     * @param  count    Size of allocated @a res array, or 0 to only count available AP
     * @param  timeout  Timeout in milliseconds; 0 for no timeout (Default: 0)
     * @return          Number of entries in @a, or if @a count was 0 number of available networks, negative on error
     *                  see @a nsapi_error
     */
    virtual int scan(WiFiAccessPoint *res, unsigned count);

    /** Translates a hostname to an IP address with specific version
     *
     *  The hostname may be either a domain name or an IP address. If the
     *  hostname is an IP address, no network transactions will be performed.
     *
     *  If no stack-specific DNS resolution is provided, the hostname
     *  will be resolve using a UDP socket on the stack.
     *
     *  @param address  Destination for the host SocketAddress
     *  @param host     Hostname to resolve
     *  @param version  IP version of address to resolve, NSAPI_UNSPEC indicates
     *                  version is chosen by the stack (defaults to NSAPI_UNSPEC)
     *  @return         0 on success, negative error code on failure
     */
    using NetworkInterface::gethostbyname;

    /** Add a domain name server to list of servers to query
     *
     *  @param addr     Destination for the host address
     *  @return         0 on success, negative error code on failure
     */
    using NetworkInterface::add_dns_server;

protected:
    /** Open a socket
     *  @param handle       Handle in which to store new socket
     *  @param proto        Type of socket to open, NSAPI_TCP or NSAPI_UDP
     *  @return             0 on success, negative on failure
     */
    virtual int socket_open(void **handle, nsapi_protocol_t proto);

    /** Close the socket
     *  @param handle       Socket handle
     *  @return             0 on success, negative on failure
     *  @note On failure, any memory associated with the socket must still
     *        be cleaned up
     */
    virtual int socket_close(void *handle);

    /** Bind a server socket to a specific port
     *  @param handle       Socket handle
     *  @param address      Local address to listen for incoming connections on
     *  @return             0 on success, negative on failure.
     */
    virtual int socket_bind(void *handle, const SocketAddress &address);

    /** Start listening for incoming connections
     *  @param handle       Socket handle
     *  @param backlog      Number of pending connections that can be queued up at any
     *                      one time [Default: 1]
     *  @return             0 on success, negative on failure
     */
    virtual int socket_listen(void *handle, int backlog);

    /** Connects this TCP socket to the server
     *  @param handle       Socket handle
     *  @param address      SocketAddress to connect to
     *  @return             0 on success, negative on failure
     */
    virtual int socket_connect(void *handle, const SocketAddress &address);

    /** Accept a new connection.
     *  @param handle       Handle in which to store new socket
     *  @param server       Socket handle to server to accept from
     *  @return             0 on success, negative on failure
     *  @note This call is not-blocking, if this call would block, must
     *        immediately return NSAPI_ERROR_WOULD_WAIT
     */
    virtual int socket_accept(void *handle, void **socket, SocketAddress *address);

    /** Send data to the remote host
     *  @param handle       Socket handle
     *  @param data         The buffer to send to the host
     *  @param size         The length of the buffer to send
     *  @return             Number of written bytes on success, negative on failure
     *  @note This call is not-blocking, if this call would block, must
     *        immediately return NSAPI_ERROR_WOULD_WAIT
     */
    virtual int socket_send(void *handle, const void *data, unsigned size);

    /** Receive data from the remote host
     *  @param handle       Socket handle
     *  @param data         The buffer in which to store the data received from the host
     *  @param size         The maximum length of the buffer
     *  @return             Number of received bytes on success, negative on failure
     *  @note This call is not-blocking, if this call would block, must
     *        immediately return NSAPI_ERROR_WOULD_WAIT
     */
    virtual int socket_recv(void *handle, void *data, unsigned size);

    /** Send a packet to a remote endpoint
     *  @param handle       Socket handle
     *  @param address      The remote SocketAddress
     *  @param data         The packet to be sent
     *  @param size         The length of the packet to be sent
     *  @return             The number of written bytes on success, negative on failure
     *  @note This call is not-blocking, if this call would block, must
     *        immediately return NSAPI_ERROR_WOULD_WAIT
     */
    virtual int socket_sendto(void *handle, const SocketAddress &address, const void *data, unsigned size);

    /** Receive a packet from a remote endpoint
     *  @param handle       Socket handle
     *  @param address      Destination for the remote SocketAddress or null
     *  @param buffer       The buffer for storing the incoming packet data
     *                      If a packet is too long to fit in the supplied buffer,
     *                      excess bytes are discarded
     *  @param size         The length of the buffer
     *  @return             The number of received bytes on success, negative on failure
     *  @note This call is not-blocking, if this call would block, must
     *        immediately return NSAPI_ERROR_WOULD_WAIT
     */
    virtual int socket_recvfrom(void *handle, SocketAddress *address, void *buffer, unsigned size);

    /** Register a callback on state change of the socket
     *  @param handle       Socket handle
     *  @param callback     Function to call on state change
     *  @param data         Argument to pass to callback
     *  @note Callback may be called in an interrupt context.
     */
    virtual void socket_attach(void *handle, void (*callback)(void *), void *data);

    /** Provide access to the NetworkStack object
     *
     *  @return The underlying NetworkStack object
     */
    virtual NetworkStack *get_stack()
    {
        return this;
    }

private:
    ESP8266 _esp;
    bool _ids[ESP8266_SOCKET_COUNT];

    struct {
        char ssid[33]; /* 32 is what 802.11 defines as longest possible name; +1 for the \0 */
        nsapi_security_t sec;
        uint8_t ch;
        char pass[64]; /* The longest allowed passphrase */
        char ip[16]; /* Static IPv4 address if not DHCP */
        char netmask[16];
        char gateway[16];
    } _sta, _ap;

    struct {
        bool enable; /* whether the dhcp server is enabled */
        char start_ip[16]; /* starting IPv4 address */
        char end_ip[16]; /* ending IPv4 address */
        int lease_time; /* lease time for dhcp clients */
    } _dhcps;

    void event();

    struct {
        void (*callback)(void *);
        void *data;
    } _cbs[ESP8266_SOCKET_COUNT];
};

#endif
