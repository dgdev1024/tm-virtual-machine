/**
 * @file  TOMBOY/Network.h
 * @brief Contains definitions and functions related to network operations in the TOMBOY emulator
 *        engine.
 */

#pragma once
#include <TOMBOY/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/** @brief The size of the TOMBOY's network RAM buffers, in bytes. */
#define TOMBOY_NETWORK_RAM_SIZE 0x100

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the TOMBOY emulator engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

/**
 * @brief A forward declaration of the TOMBOY network context structure.
 */
typedef struct TOMBOY_Network TOMBOY_Network;

// Network Status Enumeration //////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible network statuses for the TOMBOY emulator.
 */
typedef enum TOMBOY_NetworkStatus
{
    TOMBOY_NS_READY             = 0b00, ///< @brief Network is ready for communication.
    TOMBOY_NS_BUSY              = 0b01, ///< @brief Network is sending or receiving data.
    TOMBOY_NS_TIMEOUT           = 0b10, ///< @brief Network operation timed out.
    TOMBOY_NS_ERROR             = 0b11, ///< @brief Network encountered an error.
} TOMBOY_NetworkStatus;

// Network Control Union ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the TOMBOY network interface control register, `NTC`.
 */
typedef union TOMBOY_NetworkControl
{
    struct
    {
        uint8_t : 4;                ///< @brief Unused, reserved bits.
        uint8_t m_Status : 2;       ///< @brief Indicates the current status of the network operation. Read-only.
        uint8_t m_Direction : 1;    ///< @brief Indicates the direction of data transfer (1 = send, 0 = receive).
        uint8_t m_Enable : 1;       ///< @brief Indicates if a network operation has been requested or is in progress.
    };
    
    uint8_t m_Register; ///< @brief The raw register value.
} TOMBOY_NetworkControl;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new instance of the TOMBOY's network interface.
 * 
 * @param p_Engine      A pointer to the network interface's parent engine.
 * 
 * @return A pointer to the newly created TOMBOY network interface.
 */
TOMBOY_Network* TOMBOY_CreateNetwork (TOMBOY_Engine* p_Engine);

/**
 * @brief Destroys the TOMBOY network interface.
 * 
 * @param p_Network     A pointer to the network interface to destroy.
 */
void TOMBOY_DestroyNetwork (TOMBOY_Network* p_Network);

/**
 * @brief Resets the TOMBOY network interface to its initial state.
 * 
 * @param p_Network     A pointer to the network interface to reset.
 */
void TOMBOY_ResetNetwork (TOMBOY_Network* p_Network);

/**
 * @brief Ticks the given TOMBOY network interface, updating its state.
 * 
 * When the TOMBOY network interface is ticked, if a network operation is in progress, the interface
 * will transfer a new chunk of data over the network.
 * 
 * @param p_Network     A pointer to the network interface to tick.
 * 
 * @return `true` if the network operation was successful, `false` otherwise. 
 */
bool TOMBOY_TickNetwork (TOMBOY_Network* p_Network);

/**
 * @brief Connects the TOMBOY network interface to a remote host.
 * 
 * @param p_Network     A pointer to the network interface to connect.
 * @param p_Host        The hostname or IP address of the remote host.
 * @param p_Port        The port number to connect to on the remote host.
 * 
 * @return `true` if the connection was successful, `false` otherwise.
 */
bool TOMBOY_ConnectNetwork (TOMBOY_Network* p_Network, const char* p_Host, uint16_t p_Port);

/**
 * @brief Disconnects the TOMBOY network interface from the remote host.
 * 
 * @param p_Network     A pointer to the network interface to disconnect.
 */
void TOMBOY_DisconnectNetwork (TOMBOY_Network* p_Network);

// Public Function Prototypes - Memory Access //////////////////////////////////////////////////////

/**
 * @brief Reads a byte from the TOMBOY network interface's send RAM.
 * 
 * @param p_Network     A pointer to the network interface.
 * @param p_Address     The address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadNetSendByte (const TOMBOY_Network* p_Network, uint32_t p_Address);

/**
 * @brief Writes a byte to the TOMBOY network interface's send RAM.
 * 
 * @param p_Network     A pointer to the network interface.
 * @param p_Address     The address to write to.
 * @param p_Value       The value to write.
 */
void TOMBOY_WriteNetSendByte (TOMBOY_Network* p_Network, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief Reads a byte from the TOMBOY network interface's receive RAM.
 * 
 * @param p_Network     A pointer to the network interface.
 * @param p_Address     The address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadNetRecvByte (const TOMBOY_Network* p_Network, uint32_t p_Address);

// Public Function Prototypes - Hardware Register Getters //////////////////////////////////////////

/**
 * @brief Reads the value of the TOMBOY network interface's control register, `NTC`.
 * 
 * @param p_Network     A pointer to the network interface.
 * 
 * @return The value of the `NTC` register.
 */
uint8_t TOMBOY_ReadNTC (const TOMBOY_Network* p_Network);

// Public Function Prototypes - Hardware Register Setters //////////////////////////////////////////

/**
 * @brief Writes a value to the TOMBOY network interface's control register, `NTC`.
 * 
 * @param p_Network     A pointer to the network interface.
 * @param p_Value       The value to write to the `NTC` register.
 */
void TOMBOY_WriteNTC (TOMBOY_Network* p_Network, uint8_t p_Value);
