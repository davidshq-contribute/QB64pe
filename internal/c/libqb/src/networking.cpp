//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Networking Module
//  TCP/IP socket networking functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "networking.h"
#include "error_handle.h"
#include "handles.h"
#include "http.h"
#include "qblist.h"
#include "qbs.h"
#include "rounding.h"

#include "../../os.h"

#include <cstdlib>
#include <cstring>

// Platform-specific socket headers
#ifdef QB64_WINDOWS
#include <winsock2.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// ============================================================================
// EXTERNAL DEPENDENCIES
// ============================================================================

// From libqb.cpp - string functions
extern int32 func_instr(int32 start, qbs *str, qbs *substr, int32 passed);
extern qbs *func_mid(qbs *str, int32 start, int32 length, int32 passed);

// From libqb.cpp - global handles
extern list *special_handles;
extern list *stream_handles;

// From libqb.cpp - vwatch support
extern int32 vwatch;

// From main.txt (compiled QB64 code)
extern qbs *FUNC__WHATISMYIP();
extern qbs *FUNC__DECODEURL(qbs *str);

// ============================================================================
// MODULE STATE
// ============================================================================

#ifdef QB64_WINDOWS
static WSADATA wsaData;
static WORD sockVersion;
#endif

#define NETWORK_ERROR -1
#define NETWORK_OK 0

// ============================================================================
// TCP CONNECTION STRUCTURE
// ============================================================================

struct tcp_connection {
#if !defined(DEPENDENCY_SOCKETS)
#elif defined(QB64_WINDOWS)
    SOCKET socket;
#elif defined(QB64_UNIX)
    int socket;
#else
#endif
    int32 port;      // connection to host & clients only
    uint8 ip4[4];    // connection to host only
    uint8 *hostname; // clients only
    int connected;
};

// ============================================================================
// CONNECTION STRUCTURE
// ============================================================================

struct connection_struct {
    int8 in_use;   // 0=not being used, 1=in use
    int8 protocol; // 1=TCP/IP
    int8 type;     // 1=client, 2=host(listening), 3=host's connection from a client
    ptrszint stream;
    ptrszint handle;
    void *connection;
    int32 port;
};

list *connection_handles = NULL;

// ============================================================================
// NETWORKING INITIALIZATION
// ============================================================================

void networking_init() {
    connection_handles = list_new(sizeof(connection_struct));
}

// ============================================================================
// TCP INITIALIZATION
// ============================================================================

void tcp_init() {
    static int32 init = 0;
    if (!init) {
        init = 1;
#if !defined(DEPENDENCY_SOCKETS)
#elif defined(QB64_WINDOWS)
        sockVersion = MAKEWORD(1, 1);
        WSAStartup(sockVersion, &wsaData);
#endif
    }
}

void tcp_done() {
#if !defined(DEPENDENCY_SOCKETS)
#elif defined(QB64_WINDOWS)
    WSACleanup();
#endif
}

// ============================================================================
// TCP HOST OPERATIONS
// ============================================================================

static void *tcp_host_open(int64 port) {
    tcp_init();
    if ((port < 0) || (port > 65535))
        return NULL;
#if !defined(DEPENDENCY_SOCKETS)
    return NULL;
#elif defined(QB64_WINDOWS)
    static SOCKET listeningSocket;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET)
        return NULL;
    static SOCKADDR_IN serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(port);
    static int nret;
    nret = ::bind(listeningSocket, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));
    if (nret == SOCKET_ERROR) {
        closesocket(listeningSocket);
        return NULL;
    }
    nret = listen(listeningSocket, SOMAXCONN);
    if (nret == SOCKET_ERROR) {
        closesocket(listeningSocket);
        return NULL;
    }
    static u_long iMode;
    iMode = 1;
    ioctlsocket(listeningSocket, FIONBIO, &iMode);

    static tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = listeningSocket;
    connection->connected = -1;
    return (void *)connection;
#elif defined(QB64_UNIX)
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    char str_port[6];
    int yes = 1;
    snprintf(str_port, 6, "%" PRId64, port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, str_port, &hints, &servinfo) != 0)
        return NULL;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)
        return NULL;
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    if (listen(sockfd, SOMAXCONN) == -1) {
        close(sockfd);
        return NULL;
    }

    tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = sockfd;
    connection->connected = -1;
    return (void *)connection;
#else
    return NULL;
#endif
}

// ============================================================================
// TCP CLIENT OPERATIONS
// ============================================================================

static void *tcp_client_open(uint8 *host, int64 port) {
    tcp_init();

    if ((port < 0) || (port > 65535))
        return NULL;
#if !defined(DEPENDENCY_SOCKETS)
    return NULL;
#elif defined(QB64_WINDOWS)
    static LPHOSTENT hostEntry;
    hostEntry = gethostbyname((char *)host);
    if (!hostEntry)
        return NULL;
    static SOCKET theSocket;
    theSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (theSocket == INVALID_SOCKET)
        return NULL;
    static SOCKADDR_IN serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
    serverInfo.sin_port = htons(port);
    static int nret;
    nret = connect(theSocket, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));
    if (nret == SOCKET_ERROR) {
        closesocket(theSocket);
        return NULL;
    }
    static u_long iMode;
    iMode = 1;
    ioctlsocket(theSocket, FIONBIO, &iMode);

    static tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = theSocket;
    connection->port = port;
    connection->connected = -1;
    connection->hostname = (uint8 *)malloc(strlen((char *)host) + 1);
    memcpy(connection->hostname, host, strlen((char *)host) + 1);
    return (void *)connection;
#elif defined(QB64_UNIX)
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    char str_port[6];
    snprintf(str_port, 6, "%" PRId64, port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo((char *)host, str_port, &hints, &servinfo) != 0)
        return NULL;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)
        return NULL;
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = sockfd;
    connection->port = port;
    connection->connected = -1;
    connection->hostname = (uint8 *)malloc(strlen((char *)host) + 1);
    memcpy(connection->hostname, host, strlen((char *)host) + 1);
    return (void *)connection;
#else
    return NULL;
#endif
}

// ============================================================================
// TCP CONNECTION ACCEPT
// ============================================================================

static void *tcp_connection_open(void *host_tcp) {
#if !defined(DEPENDENCY_SOCKETS)
    return NULL;
#elif defined(QB64_WINDOWS)
    static tcp_connection *host;
    host = (tcp_connection *)host_tcp;
    static sockaddr sa;
    static int sa_size;
    sa_size = sizeof(sa);
    static SOCKET new_socket;
    new_socket = accept(host->socket, &sa, &sa_size);
    if (new_socket == INVALID_SOCKET)
        return NULL;
    static u_long iMode;
    iMode = 1;
    ioctlsocket(new_socket, FIONBIO, &iMode);
    static tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = new_socket;
    connection->port = *((uint16 *)sa.sa_data);
    connection->connected = -1;
    *((uint32 *)(connection->ip4)) = *((uint32 *)(sa.sa_data + 2));
    return (void *)connection;
#elif defined(QB64_UNIX)
    tcp_connection *host;
    host = (tcp_connection *)host_tcp;
    struct sockaddr remote_addr;
    socklen_t addr_size;
    int fd;

    addr_size = sizeof(remote_addr);
    fd = accept(host->socket, &remote_addr, &addr_size);
    if (fd == -1)
        return NULL;
    fcntl(fd, F_SETFL, O_NONBLOCK);

    tcp_connection *connection;
    connection = (tcp_connection *)calloc(sizeof(tcp_connection), 1);
    connection->socket = fd;
    connection->connected = -1;
    connection->port = *((uint16 *)remote_addr.sa_data);
    *((uint32 *)(connection->ip4)) = *((uint32 *)(remote_addr.sa_data + 2));
    return (void *)connection;
#else
    return NULL;
#endif
}

// ============================================================================
// TCP CLOSE AND OUTPUT
// ============================================================================

static void tcp_close(void *connection) {
    tcp_connection *tcp = (tcp_connection *)connection;
#if !defined(DEPENDENCY_SOCKETS)
#elif defined(QB64_WINDOWS)
    if (tcp->socket) {
        shutdown(tcp->socket, SD_BOTH);
        closesocket(tcp->socket);
    }
#elif defined(QB64_UNIX)
    if (tcp->socket) {
        shutdown(tcp->socket, SHUT_RDWR);
        close(tcp->socket);
    }
#endif
    if (tcp->hostname)
        free(tcp->hostname);
    free(tcp);
}

static void tcp_out(void *connection, void *offset, ptrszint bytes) {
#if !defined(DEPENDENCY_SOCKETS)
#elif defined(QB64_WINDOWS) || defined(QB64_UNIX)
#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif
    tcp_connection *tcp;
    tcp = (tcp_connection *)connection;
    int total = 0;
    int bytesleft = bytes;
    int n;

    while (total < bytes) {
        n = send(tcp->socket, (char *)((char *)offset + total), bytesleft, MSG_NOSIGNAL);
        if (n < 0) {
            tcp->connected = 0;
            return;
        }
        total += n;
        bytesleft -= n;
    }
#else
#endif
}

// ============================================================================
// STREAM OPERATIONS
// ============================================================================

void stream_free(stream_struct *st) {
    if (st->in_limit)
        free(st->in);
    list_remove(stream_handles, list_get_index(stream_handles, st));
}

void stream_out(stream_struct *st, void *offset, ptrszint bytes) {
    if (st->type == stream_type::Tcp) {
        static connection_struct *co;
        co = (connection_struct *)st->index;
        if ((co->type == 1) || (co->type == 3)) {
            if (co->protocol == 1) {
                tcp_out((void *)co->connection, offset, bytes);
            }
        }
    }
}

void stream_update(stream_struct *stream) {
#ifdef DEPENDENCY_SOCKETS
    static connection_struct *connection;
    connection = (connection_struct *)(stream->index);
    static tcp_connection *tcp;
    tcp = (tcp_connection *)(connection->connection);
    static ptrszint bytes;

    if (!stream->in_limit) {
        stream->in = (uint8 *)malloc(1024);
        stream->in_size = 0;
        stream->in_limit = 1024;
    }

expand_and_retry:
    if (stream->in_size == stream->in_limit) {
        stream->in_limit *= 2;
        stream->in = (uint8 *)realloc(stream->in, stream->in_limit);
    }

    bytes = recv(tcp->socket, (char *)(stream->in + stream->in_size), stream->in_limit - stream->in_size, 0);
    if (bytes < 0) {
#ifdef QB64_WINDOWS
        if (WSAGetLastError() != WSAEWOULDBLOCK)
            tcp->connected = 0;
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            tcp->connected = 0;
#endif
    } else if (bytes == 0) {
        tcp->connected = 0;
    } else {
        stream->in_size += bytes;
        if (stream->in_size == stream->in_limit)
            goto expand_and_retry;
    }
#endif
}

// ============================================================================
// CONNECTION CLOSE
// ============================================================================

void connection_close(ptrszint i) {
    static connection_struct *cs;
    static stream_struct *ss;
    static special_handle_struct *sh;
    sh = (special_handle_struct *)list_get(special_handles, i);
    if (!sh)
        return;

    switch (sh->type) {
    case special_handle_type::Stream:
        ss = (stream_struct *)sh->index;
        if (ss->type == stream_type::Tcp) {
            cs = (connection_struct *)ss->index;
            if (cs->protocol == 1) {
                tcp_close(cs->connection);
            }
            list_remove(connection_handles, list_get_index(connection_handles, cs));
        }
        stream_free(ss);
        break;

    case special_handle_type::Host:
        cs = (connection_struct *)sh->index;
        if (cs->protocol == 1) {
            tcp_close(cs->connection);
        }
        list_remove(connection_handles, list_get_index(connection_handles, cs));
        break;

    case special_handle_type::Http:
        libqb_http_close(i);
        break;

    case special_handle_type::Invalid:
        break;
    }
    list_remove(special_handles, i);
}

// ============================================================================
// CONNECTION NEW
// ============================================================================

int32 connection_new(int32 method, qbs *info_in, int32 value) {
    static int32 i, x;

    static qbs *info_part[10 + 1];
    static qbs *str;
    static qbs *strz;
    static qbs *info;

    static int32 first_call = 1;
    if (first_call) {
        first_call = 0;
        for (i = 1; i <= 10; i++) {
            info_part[i] = qbs_new(0, 0);
        }
        str = qbs_new(0, 0);
        strz = qbs_new(1, 0);
        strz->chr[0] = 0;
        info = qbs_new(0, 0);
    }

    static int32 parts;
    parts = 0;
    if ((method == 0) || (method == 1)) {
        qbs_set(info, info_in);
        qbs_set(str, qbs_new_txt(":"));
        i = 1;

        for (int k = 0; k < sizeof(info_part) / sizeof(*info_part) - 1; k++) {
            x = func_instr(i, info, str, 1);
            if (!x)
                break;

            parts++;
            qbs_set(info_part[parts], func_mid(info, i, x - i, 1));
            i = x + 1;
        }

        parts++;
        qbs_set(info_part[parts], func_mid(info, i, NULL, NULL));
    }

    static double d;
    static int32 port;

    if ((method == 0) || (method == 1)) {
        if (method == 0 && parts >= 1 &&
            (qbs_equal(qbs_ucase(info_part[1]), qbs_new_txt("HTTP")) || qbs_equal(qbs_ucase(info_part[1]), qbs_new_txt("HTTPS")))) {

            qbs_set(str, qbs_add(info, strz));

            const char *url = (const char *)str->chr;
            for (; *url && *url != ':'; url++)
                ;

            if (!*url)
                return -1;

            url++;

            if (*url == '/')
                url = (const char *)str->chr;

            int32 my_handle = list_add(special_handles);

            int err = libqb_http_open(url, my_handle);

            if (err) {
                list_remove(special_handles, my_handle);
                return 0;
            }

            special_handle_struct *my_handle_struct;
            my_handle_struct = (special_handle_struct *)list_get(special_handles, my_handle);

            my_handle_struct->type = special_handle_type::Http;
            my_handle_struct->index = 0;

            return my_handle;
        }

        if (parts < 2)
            return -1;

        if (qbs_equal(qbs_ucase(info_part[1]), qbs_new_txt("TCP/IP")) == 0) {
            if (qbs_equal(qbs_ucase(info_part[1]), qbs_new_txt("QB64IDE")) == 0 || vwatch != -1) {
                return -1;
            }
        }

        d = qbs_val<long double>(info_part[2]);
        port = qbr_double_to_long(d);

        if (method == 0) {
            if (parts != 3)
                return -1;

            static void *connection;
            qbs_set(str, qbs_add(info_part[3], strz));
            connection = tcp_client_open(str->chr, port);
            if (!connection)
                return 0;

            static int32 my_handle;
            my_handle = list_add(special_handles);
            static special_handle_struct *my_handle_struct;
            my_handle_struct = (special_handle_struct *)list_get(special_handles, my_handle);
            static int32 my_stream;
            my_stream = list_add(stream_handles);
            static stream_struct *my_stream_struct;
            my_stream_struct = (stream_struct *)list_get(stream_handles, my_stream);
            static int32 my_connection;
            my_connection = list_add(connection_handles);
            static connection_struct *my_connection_struct;
            my_connection_struct = (connection_struct *)list_get(connection_handles, my_connection);
            my_handle_struct->type = special_handle_type::Stream;
            my_handle_struct->index = (ptrszint)my_stream_struct;
            my_stream_struct->type = stream_type::Tcp;
            my_stream_struct->index = (ptrszint)my_connection_struct;
            my_connection_struct->protocol = 1;
            my_connection_struct->type = 1;
            my_connection_struct->connection = connection;
            my_connection_struct->port = port;

            my_stream_struct->in = NULL;
            my_stream_struct->in_size = 0;
            my_stream_struct->in_limit = 0;

            if (vwatch == -1)
                vwatch = my_handle;
            return my_handle;
        }

        if (method == 1) {
            if (parts != 2)
                return -1;

            static void *connection;
            connection = tcp_host_open(port);
            if (!connection)
                return 0;

            static int32 my_handle;
            my_handle = list_add(special_handles);
            static special_handle_struct *my_handle_struct;
            my_handle_struct = (special_handle_struct *)list_get(special_handles, my_handle);
            static int32 my_connection;
            my_connection = list_add(connection_handles);
            static connection_struct *my_connection_struct;
            my_connection_struct = (connection_struct *)list_get(connection_handles, my_connection);
            my_handle_struct->type = special_handle_type::Host;
            my_handle_struct->index = (ptrszint)my_connection_struct;
            my_connection_struct->protocol = 1;
            my_connection_struct->type = 2;
            my_connection_struct->connection = connection;
            my_connection_struct->port = port;
            return my_handle;
        }
    }

    if (method == 2) {
        static special_handle_struct *sh;
        sh = (special_handle_struct *)list_get(special_handles, value);
        if (!sh)
            return -1;
        if (sh->type != special_handle_type::Host)
            return -1;
        static connection_struct *co;
        co = (connection_struct *)sh->index;
        static void *connection;
        connection = tcp_connection_open(co->connection);
        if (!connection)
            return 0;

        static int32 my_handle;
        my_handle = list_add(special_handles);
        static special_handle_struct *my_handle_struct;
        my_handle_struct = (special_handle_struct *)list_get(special_handles, my_handle);
        static int32 my_stream;
        my_stream = list_add(stream_handles);
        static stream_struct *my_stream_struct;
        my_stream_struct = (stream_struct *)list_get(stream_handles, my_stream);
        static int32 my_connection;
        my_connection = list_add(connection_handles);
        static connection_struct *my_connection_struct;
        my_connection_struct = (connection_struct *)list_get(connection_handles, my_connection);
        my_handle_struct->type = special_handle_type::Stream;
        my_handle_struct->index = (ptrszint)my_stream_struct;
        my_stream_struct->type = stream_type::Tcp;
        my_stream_struct->index = (ptrszint)my_connection_struct;
        my_connection_struct->protocol = 1;
        my_connection_struct->type = 3;
        my_connection_struct->connection = connection;
        my_connection_struct->port = port;

        my_stream_struct->in = NULL;
        my_stream_struct->in_size = 0;
        my_stream_struct->in_limit = 0;

        return my_handle;
    }

    return -1;
}

// ============================================================================
// QB64 NETWORK FUNCTIONS
// ============================================================================

int32 func__openclient(qbs *info) {
    if (is_error_pending())
        return 0;
    static int32 i;
    i = connection_new(0, info, NULL);
    if (i == -1) {
        error(5);
        return 0;
    }
    if (i == 0)
        return 0;
    return -1 - i;
}

int32 func__openhost(qbs *info) {
    if (is_error_pending())
        return 0;
    static int32 i;
    i = connection_new(1, info, NULL);
    if (i == -1) {
        error(5);
        return 0;
    }
    if (i == 0)
        return 0;
    return -1 - i;
}

int32 func__openconnection(int32 i) {
    if (is_error_pending())
        return 0;
    i = -(i + 1);
    i = connection_new(2, NULL, i);
    if (i == -1) {
        error(258);
        return 0;
    }
    if (i == 0)
        return 0;
    return -1 - i;
}

qbs *func__connectionaddress(int32 i) {
    static qbs *tqbs, *tqbs2, *str = NULL, *str2 = NULL;
    static int32 x;
    if (is_error_pending())
        goto error;

#ifdef DEPENDENCY_SOCKETS
    if (!str)
        str = qbs_new(0, 0);
    if (!str2)
        str2 = qbs_new(0, 0);

    if (i < 0) {
        x = -(i + 1);
        static connection_struct *cs;
        static stream_struct *ss;
        static special_handle_struct *sh;
        const char *url;
        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            goto error;
        }

        switch (sh->type) {
        case special_handle_type::Stream:
            ss = (stream_struct *)sh->index;
            if (ss->type == stream_type::Tcp) {
                static connection_struct *cs;
                cs = (connection_struct *)ss->index;
                if (cs->protocol == 1) {
                    if (cs->type == 1 || cs->type == 3) {
                        static tcp_connection *tcp;
                        tcp = (tcp_connection *)cs->connection;
                        qbs_set(str, qbs_new_txt("TCP/IP:"));
                        qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(tcp->port))));
                        qbs_set(str, qbs_add(str, qbs_new_txt(":")));
                        if (cs->type == 3) {
                            qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(tcp->ip4[0]))));
                            qbs_set(str, qbs_add(str, qbs_new_txt(".")));
                            qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(tcp->ip4[1]))));
                            qbs_set(str, qbs_add(str, qbs_new_txt(".")));
                            qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(tcp->ip4[2]))));
                            qbs_set(str, qbs_add(str, qbs_new_txt(".")));
                            qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(tcp->ip4[3]))));
                        } else {
                            qbs_set(str, qbs_add(str, qbs_new_txt((char *)tcp->hostname)));
                        }
                        return str;
                    }
                }
            }
            break;

        case special_handle_type::Host:
            cs = (connection_struct *)sh->index;
            if (cs->protocol == 1) {
                qbs_set(str, qbs_new_txt("TCP/IP:"));
                qbs_set(str, qbs_add(str, qbs_ltrim(qbs_str(cs->port))));
                qbs_set(str, qbs_add(str, qbs_new_txt(":")));
                tqbs2 = FUNC__WHATISMYIP();
                if (tqbs2->len) {
                    qbs_set(str, qbs_add(str, tqbs2));
                } else {
                    qbs_set(str, qbs_add(str, qbs_new_txt("127.0.0.1")));
                }
                return str;
            }
            break;

        case special_handle_type::Http:
            url = libqb_http_get_url(x);

            qbs_set(str, qbs_new_txt("HTTP:"));

            if (url)
                qbs_set(str, qbs_add(str, qbs_new_txt(url)));
            else
                qbs_set(str, qbs_add(str, qbs_new_txt("UNKNOWN")));

            return FUNC__DECODEURL(str);

        case special_handle_type::Invalid:
            break;
        }
    }
#endif
    error(52);

error:
    tqbs = qbs_new(0, 1);
    return tqbs;
}

int32 tcp_connected(void *connection) {
    tcp_connection *tcp = (tcp_connection *)connection;
#ifndef DEPENDENCY_SOCKETS
    return 0;
#else
    return tcp->connected;
#endif
}

int32 func__connected(int32 i) {
    if (is_error_pending())
        return 0;
    if (i < 0) {
        static int32 x;
        x = -(i + 1);
        static stream_struct *ss;
        static connection_struct *cs;
        static special_handle_struct *sh;
        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh)
            goto error;

        switch (sh->type) {
        case special_handle_type::Stream:
            ss = (stream_struct *)sh->index;
            if (ss->type == stream_type::Tcp) {
                cs = (connection_struct *)ss->index;
                if (cs->protocol == 1) {
                    return tcp_connected(cs->connection);
                }
            }
            break;

        case special_handle_type::Host:
            cs = (connection_struct *)sh->index;
            if (cs->protocol == 1) {
                return -1;
            }
            break;

        case special_handle_type::Http:
            return libqb_http_connected(x) ? -1 : 0;

        case special_handle_type::Invalid:
            break;
        }
    }
error:
    error(52);
    return 0;
}
