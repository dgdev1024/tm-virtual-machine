/**
 * @file  TOMBOY/Network.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/Network.h>

// Platform-Specific Includes
#if defined(TM_LINUX)
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <errno.h>

    #define TOMBOY_INVALID_SOCKET -1
    #define TOMBOY_SOCKET_ERROR   -1

    typedef int TOMBOY_Socket;
#else
    #error "The TOMBOY emulator engine's network interface is not yet implemented for this platform."
#endif

// TOMBOY Network Interface Structure //////////////////////////////////////////////////////////////

typedef struct TOMBOY_Network
{

    // Parent Engine
    TOMBOY_Engine* m_ParentEngine;      ///< @brief A pointer to the parent TOMBOY engine.

    // Buffers
    uint8_t m_SendBuffer[TOMBOY_NETWORK_RAM_SIZE]; ///< @brief The send buffer for network data.
    uint8_t m_RecvBuffer[TOMBOY_NETWORK_RAM_SIZE]; ///< @brief The receive buffer for network data.

    // Hardware Registers
    TOMBOY_NetworkControl m_NTC;        ///< @brief The network control register.

    // Internal State
    uint16_t m_BytesLeft;              ///< @brief The number of bytes left to send or receive.
    uint16_t m_BytesTransferred;       ///< @brief The number of bytes transferred in the current operation.
    uint32_t m_Timeout;                ///< @brief The timeout value for network operations.
    uint32_t m_TimeoutCounter;         ///< @brief The current timeout counter.

    // Socket
    TOMBOY_Socket m_Socket;            ///< @brief The socket used for network communication.

} TOMBOY_Network;

// Private Functions ///////////////////////////////////////////////////////////////////////////////

static void TOMBOY_PerformNetworkTransfer (TOMBOY_Network* p_Network)
{
    // Check to see if there is a network operation in progress.
    if (p_Network->m_NTC.m_Status != TOMBOY_NS_BUSY)
    {
        return;
    }

    // Don't bother performing a network transfer if the socket is invalid.
    if (p_Network->m_Socket == TOMBOY_INVALID_SOCKET)
    {
        TM_error("Cannot perform network transfer: invalid socket.");
        p_Network->m_NTC.m_Status = TOMBOY_NS_ERROR;
        p_Network->m_NTC.m_Enable = 0;

        TOMBOY_RequestInterrupt(p_Network->m_ParentEngine, TOMBOY_IT_NET);
        return;
    }

#if defined(TM_LINUX)

    // Depending on the direction, either send or receive data.
    ssize_t l_BytesTransferred = 0;
    if (p_Network->m_NTC.m_Direction == 1)
    {
        // Send data in a non-blocking manner.
        l_BytesTransferred = send(p_Network->m_Socket, p_Network->m_SendBuffer + p_Network->m_BytesTransferred, 
            p_Network->m_BytesLeft, MSG_DONTWAIT);
    }
    else
    {
        // Receive data in a non-blocking manner.
        l_BytesTransferred = recv(p_Network->m_Socket, p_Network->m_RecvBuffer + p_Network->m_BytesTransferred, 
            p_Network->m_BytesLeft, MSG_DONTWAIT);
    }

    // Check for errors.
    if (l_BytesTransferred < 0)
    {
        // If the error is EAGAIN or EWOULDBLOCK, it means that no data was sent or received.
        // In this case, we just increment the timeout counter (next block), then try again on the
        // next tick.
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            l_BytesTransferred = 0; // No data available
        }
        else
        {
            TM_perror("Error during network transfer");
            p_Network->m_NTC.m_Status = TOMBOY_NS_ERROR;
            p_Network->m_NTC.m_Enable = 0;

            // If the error was due to a disconnected socket, close it.
            if (errno == ENOTCONN || errno == ECONNRESET)
            {
                TOMBOY_DisconnectNetwork(p_Network);
            }

            TOMBOY_RequestInterrupt(p_Network->m_ParentEngine, TOMBOY_IT_NET);
            return;
        }
    }

    // If no bytes were transferred, check for timeout.
    if (l_BytesTransferred == 0)
    {
        p_Network->m_TimeoutCounter++;
        if (p_Network->m_TimeoutCounter >= p_Network->m_Timeout)
        {
            // Timeout occurred
            p_Network->m_NTC.m_Status = TOMBOY_NS_TIMEOUT;
            p_Network->m_NTC.m_Enable = 0;
            
            TOMBOY_RequestInterrupt(p_Network->m_ParentEngine, TOMBOY_IT_NET);
        }

        return;
    }

    // Update the number of bytes left to send or receive.
    p_Network->m_BytesTransferred += l_BytesTransferred;
    p_Network->m_BytesLeft -= l_BytesTransferred;
    if (p_Network->m_BytesLeft == 0 || p_Network->m_BytesTransferred >= TOMBOY_NETWORK_RAM_SIZE)
    {
        // Network operation completed successfully.
        p_Network->m_NTC.m_Status = TOMBOY_NS_READY;
        p_Network->m_NTC.m_Enable = 0;

        TOMBOY_RequestInterrupt(p_Network->m_ParentEngine, TOMBOY_IT_NET);
        return;
    }

#endif
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Network* TOMBOY_CreateNetwork (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Parent engine pointer is NULL.");
        return NULL;
    }

    // Allocate memory for the network interface
    TOMBOY_Network* p_Network = TM_calloc(1, TOMBOY_Network);
    TM_pexpect(p_Network != NULL, "Failed to allocate memory for TOMBOY network interface");

    // Initialize the network interface
    TOMBOY_ResetNetwork(p_Network);
    p_Network->m_ParentEngine = p_Engine;

    // Initialize the socket to an invalid state
    p_Network->m_Socket = TOMBOY_INVALID_SOCKET;

    return p_Network;
}

void TOMBOY_DestroyNetwork (TOMBOY_Network* p_Network)
{
    if (p_Network != NULL)
    {
        TOMBOY_DisconnectNetwork(p_Network);
        p_Network->m_ParentEngine = NULL;
        TM_free(p_Network);
    }
}

void TOMBOY_ResetNetwork (TOMBOY_Network* p_Network)
{
    if (p_Network == NULL)
    {
        TM_error("Network pointer is NULL.");
        return;
    }

    // Reset the network control register to its default state.
    p_Network->m_NTC.m_Register = 0;

    // Reset the internal state of the network interface.
    p_Network->m_BytesLeft = 0;
    p_Network->m_BytesTransferred = 0;
    p_Network->m_Timeout = 65536;
    p_Network->m_TimeoutCounter = 0;
}

bool TOMBOY_TickNetwork (TOMBOY_Network* p_Network)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface is NULL.");
        return false;
    }

    // Check if a network operation is in progress.
    if (p_Network->m_NTC.m_Enable == true && p_Network->m_NTC.m_Status == TOMBOY_NS_BUSY)
    {
        TOMBOY_PerformNetworkTransfer(p_Network);
    }

    return true;
}

bool TOMBOY_ConnectNetwork (TOMBOY_Network* p_Network, const char* p_Host, uint16_t p_Port)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return false;
    }

    // Ensure the hostname is not NULL or blank.
    if (p_Host == NULL || p_Host[0] == '\0')
    {
        TM_error("Hostname is NULL or blank.");
        return false;
    }

    // Ensure the port is valid.
    if (p_Port == 0)
    {
        TM_error("Port number cannot be zero.");
        return false;
    }

    // If the socket is already connected, close it first.
    if (p_Network->m_Socket != TOMBOY_INVALID_SOCKET)
    {
        close(p_Network->m_Socket);
        p_Network->m_Socket = TOMBOY_INVALID_SOCKET;
    }

#if defined(TM_LINUX)
    // In case `p_Host` is a hostname, resolve it to an IP address with `getaddrinfo`.
    struct addrinfo     l_AddressHints = { 0 };
    struct addrinfo*    l_AddressInfo = NULL;
    int l_Result = getaddrinfo(p_Host, NULL, &l_AddressHints, &l_AddressInfo);
    if (l_Result != 0)
    {
        TM_error("Failed to resolve hostname: %s", gai_strerror(l_Result));
        return false;
    }

    // Iterate through the address info structures returned, and look for the best match.
    struct addrinfo* l_CurrentAddress = l_AddressInfo;
    while (l_CurrentAddress != NULL)
    {
        // Address must be IPv6 and TCP.
        if (
            l_CurrentAddress->ai_family != AF_INET6 ||
            l_CurrentAddress->ai_socktype != SOCK_STREAM ||
            l_CurrentAddress->ai_protocol != IPPROTO_TCP
        )
        {
            l_CurrentAddress = l_CurrentAddress->ai_next;
            continue;
        }

        // Try to create a socket.
        p_Network->m_Socket = socket(l_CurrentAddress->ai_family, l_CurrentAddress->ai_socktype, 
            l_CurrentAddress->ai_protocol);
        if (p_Network->m_Socket == TOMBOY_INVALID_SOCKET)
        {
            l_CurrentAddress = l_CurrentAddress->ai_next;
            continue;
        }

        break;
    }

    // If no valid address was found, free the address info and return an error.
    if (p_Network->m_Socket == TOMBOY_INVALID_SOCKET)
    {
        TM_perror("Failed to create socket");
        freeaddrinfo(l_AddressInfo);
        return false;
    }

    // Prepare the address structure for the connection.
    struct sockaddr_in6 l_SocketAddress = { 0 };
    l_SocketAddress.sin6_family = AF_INET6;
    l_SocketAddress.sin6_port = htons(p_Port);
    memcpy(&l_SocketAddress.sin6_addr, l_CurrentAddress->ai_addr, l_CurrentAddress->ai_addrlen);
    freeaddrinfo(l_AddressInfo);

    // Attempt to connect to the server.
    l_Result = connect(p_Network->m_Socket, (struct sockaddr*) &l_SocketAddress, sizeof(l_SocketAddress));
    if (l_Result == TOMBOY_SOCKET_ERROR)
    {
        TM_perror("Failed to connect to server");
        close(p_Network->m_Socket);
        p_Network->m_Socket = TOMBOY_INVALID_SOCKET;
        return false;
    }
#endif

    return true;
}

void TOMBOY_DisconnectNetwork (TOMBOY_Network* p_Network)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return;
    }

#if defined(TM_LINUX)

    // Close the socket if it is valid
    if (p_Network->m_Socket != TOMBOY_INVALID_SOCKET)
    {
        close(p_Network->m_Socket);
        p_Network->m_Socket = TOMBOY_INVALID_SOCKET;
    }

#endif
}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

uint8_t TOMBOY_ReadNetSendByte (const TOMBOY_Network* p_Network, uint32_t p_Address)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return 0xFF;
    }

    // Network send RAM is inaccessible if a network send operation is in progress.
    if (
        p_Network->m_NTC.m_Status == TOMBOY_NS_BUSY &&
        p_Network->m_NTC.m_Direction == 1
    )
    {
        return 0xFF;
    }

    return p_Network->m_SendBuffer[p_Address - TOMBOY_NSEND_START];
}

void TOMBOY_WriteNetSendByte (TOMBOY_Network* p_Network, uint32_t p_Address, uint8_t p_Value)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return;
    }

    // Network send RAM is inaccessible if a network send operation is in progress.
    if (
        p_Network->m_NTC.m_Status == TOMBOY_NS_BUSY &&
        p_Network->m_NTC.m_Direction == 1
    )
    {
        return;
    }

    p_Network->m_SendBuffer[p_Address - TOMBOY_NSEND_START] = p_Value;
}

uint8_t TOMBOY_ReadNetRecvByte (const TOMBOY_Network* p_Network, uint32_t p_Address)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return 0xFF;
    }

    // Network receive RAM is inaccessible if a network receive operation is in progress.
    if (
        p_Network->m_NTC.m_Status == TOMBOY_NS_BUSY &&
        p_Network->m_NTC.m_Direction == 0
    )
    {
        return 0xFF;
    }

    return p_Network->m_RecvBuffer[p_Address - TOMBOY_NRECV_START];
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadNTC (const TOMBOY_Network* p_Network)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return 0xFF;
    }

    return p_Network->m_NTC.m_Register;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteNTC (TOMBOY_Network* p_Network, uint8_t p_Value)
{
    if (p_Network == NULL)
    {
        TM_error("Network interface pointer is NULL.");
        return;
    }

    // The NTC register is read-only during a network operation.
    if (p_Network->m_NTC.m_Status == TOMBOY_NS_BUSY)
    {
        return;
    }

    // Update the NTC register with the new value.
    p_Network->m_NTC.m_Register = p_Value;

    // If the enable bit is set, start the network operation.
    if (p_Network->m_NTC.m_Enable == 1)
    {
        p_Network->m_NTC.m_Status = TOMBOY_NS_BUSY;
        p_Network->m_BytesLeft = TOMBOY_NETWORK_RAM_SIZE;
        p_Network->m_BytesTransferred = 0;
        p_Network->m_TimeoutCounter = 0;
        p_Network->m_Timeout = 65536; // Default timeout value
    }
}
