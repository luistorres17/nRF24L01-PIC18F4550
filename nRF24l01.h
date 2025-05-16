/* 
 * File:   nRF24l01.h
 * Author: Luis
 *
 * Created on 15 de mayo de 2025, 12:53 AM
 */

#define CE LATAbits.LATA2
#define CSN LATAbits.LATA3
#define IRQ PORTBbits.RB2

// nRF24L01 SPI Instruction Set  
//  Name  Instruction Description   
#define R_REGISTER       0b00000000      // Read a nRF24L01 Register (Register Address:  
#define W_REGISTER  0b00100000      // Write a nRF24L01 Register (Register Address: 
#define R_RX_PAYLOAD     0b01100001      // Read RX Payload (used in RX Mode) 
#define W_TX_PAYLOAD    0b10100000      // Write TX Payload (used in TX Mode) 
#define FLUSH_RX        0b11100010      // Flush RX FIFO (RX Payload) (used in RX Mode) 
#define FLUSH_TX         0b11100001      // Flush TX FIFO (TX Payload) (used in TX Mode) 
#define NOP 0b11111111 // No Operation. Used to get nRF24L01 Status.
            
// nRF24L01 Register Definitions        
//  Name  Address Description      
#define CONFIG           0x00      // General Configuration Register 
#define EN_AA             0x01       // Enable Auto Acknowledgment Function 
#define EN_RXADDR        0x02       // Enabled RX Addresses          
#define SETUP_AW         0x03       // Setup of Address Widths 
#define SETUP_RETR       0x04       // Setup of Automatic Retransmission 
#define RF_CH            0x05       // RF Channel 
#define RF_SETUP        0x06       // RF Setup Register 
#define STATUS          0x07       // nRF24L01 STATUS REGISTER 
#define OBSERVE_TX      0x08       // Transmit observe register 
#define CD              0x09       // Carrier Detect register 
#define RX_ADDR_P0        0x0A       // Receive Address for Data Pipe 0  (3, 4 or 5 
#define RX_ADDR_P1        0x0B       // Receive Address for Data Pipe 1  (3, 4 or 5 
#define RX_ADDR_P2    0x0C        // Receive Address for Data Pipe 2  (1 byte) (used 
#define RX_ADDR_P3     0x0D       // Receive Address for Data Pipe 3  (1 byte) (used 
#define RX_ADDR_P4      0x0E       // Receive Address for Data Pipe 4  (1 byte) (used 
#define RX_ADDR_P5      0x0F       // Receive Address for Data Pipe 5  (1 byte) (used 
#define TX_ADDR        0x10       // Transmit Address   (3, 4 or 5 byte lenght) 
#define RX_PW_P0        0x11       // RX FIFO Memory Width reserved for Pipe 0 
#define RX_PW_P1         0x12       // RX FIFO Memory Width reserved for Pipe 1 
#define RX_PW_P2         0x13       // RX FIFO Memory Width reserved for Pipe 2 
#define RX_PW_P3         0x14       // RX FIFO Memory Width reserved for Pipe 3 
#define RX_PW_P4         0x15        // RX FIFO Memory Width reserved for Pipe 4 
#define RX_PW_P5         0x16        // RX FIFO Memory Width reserved for Pipe 5 
#define FIFO_STATUS       0x17       // FIFO Status Register 
#define MASK_RX_DR 0b01000000 // Mask interrupt caused by RX_RD 
#define MASK_TX_DS 0b00100000 // Mask interrupt caused by TX_DS 
#define MASK_MAX_RT 0b00010000 // Mask interrupt caused by MAX_RT 
#define EN_CRC 0b00001000 // Enable CRC 
#define CRCO 0b00000100 // CRC Width: 0 => 1 byte, 1 => 2 byte 
#define PWR_UP 0b00000010 // 1: POWER UP, 0:POWER DOWN 
#define PRIM_RX 0b00000001 // 1: RX Mode, 0: TX Mode
// nRF24L01 STATUS Register BIT Definition         
//  Name  Bit Description 
#define RX_DR 0b01000000 // Data Ready RX FIFO interrupt => Set high when 
#define TX_DS 0b00100000 // Data Sent TX FIFO interrupt => Set high when 
#define MAX_RT 0b00010000 // Maximum number of TX retries interrupt (Write 1 
#define RX_P_NO 0b00001110 // Data Pipe Number of the Payload Available for 
#define PRIM_RX 0b00000001 // TX FIFO full flag


void SPI_Start (unsigned char Clock_Frequency);
void nRF24L01_Ports_Start(void); 
unsigned char SPI_Transfer(unsigned char byte_to_send); 
unsigned char Read_nRF24L01_Register (unsigned char Register_Address);
void Write_nRF24L01_Register ( unsigned char Register_Address, unsigned char Register_Content ); 
unsigned char Read_nRF24L01_Status (void);  
void Write_nRF24L01_Status (unsigned char Register_Content);
void Read_nRF24L01_Address_Register (  unsigned char TX_RX_Address_Width, unsigned char Register_Address,  unsigned char *Register_Content );
void Write_nRF24L01_Address_Register ( unsigned char TX_RX_Address_Width, unsigned char Register_Address,  unsigned char *Register_Content );
unsigned char Read_nRF24L01_RX_Payload ( unsigned char Enable_Checksum, unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload );
void Write_nRF24L01_TX_Payload (  unsigned char Enable_Checksum, unsigned char TX_RX_Payload_Width,  unsigned char *TX_Payload ); 
void Reset_nRF24L01_Status_and_nRF24L01_Payloads (void); 
void Start_RX_Mode_nRF24L01 (unsigned char TX_RX_Address_Width,  unsigned char Frequency_Channel,  unsigned char RF_Data_Rate,    unsigned char RF_Output_Power,  unsigned char LNA_Gain,   unsigned char CRC_Setup,  unsigned char *RX_Pipe0_Address,   unsigned char *RX_Pipe1_Address, unsigned char RX_Pipe2_Address, unsigned char Rx_Pipe3_Address, unsigned char Rx_Pipe4_Address, unsigned char Rx_Pipe5_Address, unsigned char Enable_Checksum, unsigned char RX_P0_Payload_Width, unsigned char RX_P1_Payload_Width, unsigned char RX_P2_Payload_Width, unsigned char RX_P3_Payload_Width, unsigned char RX_P4_Payload_Width, unsigned char RX_P5_Payload_Width );
void Start_TX_Mode_nRF24L01 ( unsigned char Frequency_Channel, unsigned char RF_Output_Power, unsigned char CRC_Setup, unsigned char Max_Auto_Retransmit, unsigned char TX_RX_Address_Width, unsigned char RF_Data_Rate, unsigned char LNA_Gain, unsigned char Auto_Retransmit_Delay, unsigned char Enable_Checksum, unsigned char TX_RX_Payload_Width);
unsigned char Receive_Data_RX_Mode_nRF24L01( unsigned char Enable_Checksum,  unsigned char Max_Waiting_ds,  unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload );
void Send_Data_TX_Mode_nRF24L01( unsigned char Enable_Checksum,unsigned char TX_RX_Address_Width,unsigned char *TX_Address,unsigned char TX_RX_Payload_Width, unsigned char *TX_Payload );
unsigned char Check_Data_Sent_TX_Mode_nRF24L01 (void);
void Finish_nRF24L01_Operation (void);
void Finish_SPI_Operation (void);