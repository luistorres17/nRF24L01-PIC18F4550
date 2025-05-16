#include <xc.h>
#include <stdint.h>
#include "nRF24l01.h"
#include "config.h"


void SPI_Start (unsigned char Clock_Frequency)  
{  
    TRISBbits.RB1 = 0;      //RB1 -> SCK
    TRISBbits.RB0 = 1;      //RB0 -> SDI(MISO)
    TRISCbits.RC7 = 0;      //RC7 -> SDO(MOSI)
    SSPCON1 = (SSPCON1 & 0xF0) | Clock_Frequency;
    SSPCON1bits.CKP=0;  // Sets SPI mode: For the nRF24L01 => Clock Polarity(CPOL)=0 
    SSPSTATbits.CKE=1; // Sets SPI mode: For the nRF24L01 =>  
    SSPSTATbits.SMP=1; // Sets SPI mode:    For the nRF24L01 => Sample Bit (SMP) = 1 
    PIE1bits.SSPIE=0; // SPI Interruptions configuration:  
    IPR1bits.SSPIP=0; // SPI Interruptions configuration:  
    PIR1bits.SSPIF=0; // SPI Interruptions configuration: 
    SSPCON1bits.SSPEN=1;   // Enable SPI Communication  
}

void nRF24L01_Ports_Start(void) 
{ 
    ADCON1=0x0F;  // As PORTA bits 2 to 4 are used as Digital Ports, ADCON1 can be only 0x0F, 0x0E or 0x0D 
    TRISAbits.TRISA2=0; // CE is an Output Pin 
    TRISAbits.TRISA3=0;  // CSN is an Output Pin  
    TRISBbits.TRISB2=1;  // IRQ is an Input Pin 
    CSN=1;  // At program start CSN must be set High (Inactive) => Signal used for Starting a new SPI Communication 
    CE=0;  // At program start CE must be set Low (Inactive) => Signal used for turning Standby into RX or TX Mode 
}   


unsigned char SPI_Transfer(unsigned char byte_to_send)   
{ 
    SSPBUF = byte_to_send;  // Byte to Send is loaded in the SSP Buffer, when nRF24L01 is ready it will be exchanged for an  Incoming Byte 

    while(PIR1bits.SSPIF==0); // Wait until exchange is done 
    PIR1bits.SSPIF=0;  // Clear SPI Interrupt Flag 

    return(SSPBUF);  // We return the Received Byte 
}

unsigned char Read_nRF24L01_Register (unsigned char Register_Address)  
{ 
    unsigned char Register_Content, nRF24L01_Status; 

    CSN=0;   // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(R_REGISTER+Register_Address);   
       // Send Instruction Word while Receiving nRF24L01_Status 
    Register_Content=SPI_Transfer(0x00);  
        // Receive Data Byte while sending dummydata (ignored by the nRF24L01) 
    CSN=1;    // DEACTIVATE CSN 
    return(Register_Content); // RETURN Register Content 
} 

void Write_nRF24L01_Register (unsigned char Register_Address,unsigned char Register_Content )
{
    unsigned char dummydata, nRF24L01_Status;
    CSN=0;   // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(W_REGISTER+Register_Address);   
    // Send Instruction Word while Receiving nRF24L01_Status  
    dummydata=SPI_Transfer(Register_Content);      
    // Send Data Byte while receiving dummydata  (discarded by the PIC18)
    CSN=1;   // DEACTIVATE CSN
    while(Read_nRF24L01_Register(Register_Address)!=Register_Content)
    {
        CSN=0; 
        nRF24L01_Status=SPI_Transfer(W_REGISTER+Register_Address);  
        dummydata=SPI_Transfer(Register_Content);   
        CSN=1; 
    }
    
}

unsigned char Read_nRF24L01_Status (void)     
{ 
    unsigned char nRF24L01_Status; 

    CSN=0;     // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(NOP);   // Send Instruction Word while Receiving nRF24L01_Status 
    CSN=1;     // DEACTIVATE CSN 

    return(nRF24L01_Status);   
} 

void Write_nRF24L01_Status (unsigned char Register_Content)      
{ 
    unsigned char dummydata, nRF24L01_Status; 

    CSN=0;    // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(W_REGISTER+STATUS);       
        // Send Instruction Word (Write STATUS Register) while Receiving nRF24L01_Status  
    dummydata=SPI_Transfer(Register_Content);      
        // Send Data Byte while receiving dummydata (discarded by the PIC18) 
    CSN=1;    // DEACTIVATE CSN 
} 

void Read_nRF24L01_Address_Register (  unsigned char TX_RX_Address_Width,  unsigned char Register_Address,  unsigned char *Register_Content )  
{ 
    unsigned char i, nRF24L01_Status; 

    CSN=0;   // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(R_REGISTER+Register_Address);     
    for(i=0; i<(TX_RX_Address_Width+0b10); i++)
    {
        Register_Content[i]=SPI_Transfer(0x00); 
    }
    CSN=1;
}

void Write_nRF24L01_Address_Register ( unsigned char TX_RX_Address_Width,  unsigned char Register_Address,  unsigned char *Register_Content )
{
    unsigned char dummydata, i, nRF24L01_Status; 
    unsigned char Process_Finished;  
    unsigned char Address_Verification[5];      
    
    Process_Finished=0;  // At funtion start the variable is set low, so  process is not finished yet    
    while(!Process_Finished)
    {
        CSN=0;   // ACTIVATE CSN 
        nRF24L01_Status=SPI_Transfer(W_REGISTER+Register_Address);
        for(i=0; i<(TX_RX_Address_Width+0b10); i++)
        {
            dummydata=SPI_Transfer(Register_Content[i]);
        }
        CSN=1;   // DEACTIVATE CSN 
 
        Read_nRF24L01_Address_Register(TX_RX_Address_Width, Register_Address, Address_Verification); // The address is read to be sure it was well writen 
        Process_Finished=1;    
        for(i=0; i<(TX_RX_Address_Width+0b10); i++)
        {
            if(Register_Content[i]!=Address_Verification[i])    
                { 
                 Process_Finished=0;  
                 // When there is a mismatch between them, the variable is set low in order to repeat process 
                } 
        }
    }
}

 
unsigned char Read_nRF24L01_RX_Payload ( unsigned char Enable_Checksum, unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload  ) 
{
    unsigned char nRF24L01_Status, TX_Checksum;      
  // Two Checksums are defined: TX_Checksum is the one received from TX Device 
    unsigned char RX_Checksum, Checksum_Conclusion;     
  // RX_Checksum is the one calculated by the RX Device. Both are compared to  verify transmission.  
    unsigned char i;         
        
    CSN=0;  // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(R_RX_PAYLOAD);  // Send Instruction Word while
    
    if(!Enable_Checksum) // Operation in case that Chechsum Byte is NOT used 
    { 
        for(i=0; i<TX_RX_Payload_Width; i++)  // For the whole Payload Width 
        { 
         RX_Payload[i]=SPI_Transfer(0x00);     
           // Receive Data Bytes while sending dummydata  (ignored by the nRF24L01)  
        } 
        Checksum_Conclusion=1;  // RX_Payload is assumed to be correct 
    }
    else if(Enable_Checksum) // Operation in case that Chechsum Byte is used 
    { 
        RX_Checksum=0b00000000; 
        for(i=0; i<TX_RX_Payload_Width; i++) // For the whole Payload Width 
        { 
            RX_Payload[i]=SPI_Transfer(0x00);     
                // Receive Data Bytes while sending dummydata (ignored by the nRF24L01)  
            RX_Checksum=RX_Checksum+RX_Payload[i];    
                // Update RX Checksum byte with every Data Byte received 
        } 
        TX_Checksum=SPI_Transfer(0x00); // Receive Checksum Byte from TX Device(an extra bit for Receiveing Payload) 
        if(RX_Checksum==TX_Checksum) 
        { 
            Checksum_Conclusion=1;  // RX_Payload is assumed to be correct 
        } 
        else if(RX_Checksum!=TX_Checksum) 
        { 
            Checksum_Conclusion=0;  // RX_Payload is assumed NOT to  be correct 
        } 
    } 
    CSN=1;   // DEACTIVATE CSN 
 
    return Checksum_Conclusion;
}

void Write_nRF24L01_TX_Payload (  unsigned char Enable_Checksum,  unsigned char TX_RX_Payload_Width,  unsigned char *TX_Payload  )  
{ 
    unsigned char nRF24L01_Status, TX_Checksum; 
    unsigned char i, dummydata; 

    CSN=0;      // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(W_TX_PAYLOAD); // Send Instruction Word while  Receiving nRF24L01_Status 
    if(!Enable_Checksum)    // Operation in case that Chechsum  Byte is NOT used 
    { 
        for(i=0; i<TX_RX_Payload_Width; i++) // For the whole Payload Width 
        { 
            dummydata=SPI_Transfer(TX_Payload[i]); 
            // Send Data Byte while receiving dummydata  (discarded by the PIC18) 
        } 
    }  
    else if(Enable_Checksum) // Operation in case that Chechsum Byte is used 
    { 
        TX_Checksum=0b00000000; 
        for(i=0; i<TX_RX_Payload_Width; i++)  // For the whole Payload Width 
        { 
            dummydata=SPI_Transfer(TX_Payload[i]); 
            // Send Data Byte while receiving dummydata (discarded by the PIC18) 
            TX_Checksum=TX_Checksum+TX_Payload[i];    
               // Update Checksum byte with every Data Byte form TX_Payload is sent 
        }  
        dummydata=SPI_Transfer(TX_Checksum);      
        // Send Checksum Byte Result, an extra bit for  Sending Payload 
    } 
    CSN=1;    // DEACTIVATE CSN 
}

void Reset_nRF24L01_Status_and_nRF24L01_Payloads (void)    
{ 
    unsigned char nRF24L01_Status; 

    CSN=0;      // ACTIVATE CSN 
    nRF24L01_Status=SPI_Transfer(FLUSH_TX); // Send Instruction to Flush TX while  receiving nRF24L01 Status 
    CSN=1;      // DEACTIVATE CSN 
    __delay_us(1);  // Retardo mínimo aproximado seguro 
    CSN=0;      // ACTIVATE CSN  
    nRF24L01_Status=SPI_Transfer(FLUSH_RX); // Send Instruction to Flush RX while receiving nRF24L01 Status
    CSN=1;      // DEACTIVATE CSN 
 
    Write_nRF24L01_Status(RX_DR);   // Delete RX_DR flag if it is High  
    Write_nRF24L01_Status(TX_DS);   // Delete TX_DS flag if it is High 
    Write_nRF24L01_Status(MAX_RT);  // Delete MAX_RT flag if it is High 
} 

void Start_RX_Mode_nRF24L01 (   unsigned char TX_RX_Address_Width,  unsigned char Frequency_Channel,  unsigned char RF_Data_Rate,    unsigned char RF_Output_Power,  unsigned char LNA_Gain,  unsigned char CRC_Setup,  unsigned char *RX_Pipe0_Address,   unsigned char *RX_Pipe1_Address, unsigned char RX_Pipe2_Address,   unsigned char RX_Pipe3_Address,  unsigned char RX_Pipe4_Address, unsigned char RX_Pipe5_Address,  unsigned char Enable_Checksum, unsigned char RX_P0_Payload_Width, unsigned char RX_P1_Payload_Width,  unsigned char RX_P2_Payload_Width, unsigned char RX_P3_Payload_Width,  unsigned char RX_P4_Payload_Width, unsigned char RX_P5_Payload_Width ) 
{
   CE=0;    // We rest in Standby I Mode (Low Consumption) until
   Reset_nRF24L01_Status_and_nRF24L01_Payloads();     
   // Reset the nRF24L01 Status Register and Flush TX and RX Payload Memories  
   Write_nRF24L01_Register (EN_AA, 0b00111111);      
    // Enable Auto Acknowledgement for all RX Pipes 
   Write_nRF24L01_Register (EN_RXADDR, 0b00111111);     
    // Enable all RX Pipes  
   Write_nRF24L01_Register (SETUP_AW, TX_RX_Address_Width);    
    // Set Width for all Addresses: 0b01 => 3 bytes, 0b10 => 4 bytes, 0b11 => 5 bytes  
   Write_nRF24L01_Register (RF_CH, Frequency_Channel);     
    // Set the frequency channel nRF24L01 operates on  
  Write_nRF24L01_Register (RF_SETUP, RF_Data_Rate*0b1000 + RF_Output_Power*0b10 + 
  LNA_Gain); // Set nRF24L01 RF Data Rate, Power and LNA Gain 
   Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P0, RX_Pipe0_Address);
    // Define RX Address for Pipe0 (3, 4 or 5 bytes) 
   Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P1, RX_Pipe1_Address);
    // Define RX Address for Pipe1 (3, 4 or 5 bytes) 
   Write_nRF24L01_Register (RX_ADDR_P2, RX_Pipe2_Address);    
    // Define RX Address for Pipe2 (1 byte => MSBytes will be equal to RX_ADDR_P1) 
   Write_nRF24L01_Register (RX_ADDR_P3, RX_Pipe3_Address);    
    // Define RX Address for Pipe3 (1 byte => MSBytes will be equal to RX_ADDR_P1) 
   Write_nRF24L01_Register (RX_ADDR_P4, RX_Pipe4_Address);    
    // Define RX Address for Pipe4 (1 byte => MSBytes will be equal to RX_ADDR_P1) 
   Write_nRF24L01_Register (RX_ADDR_P5, RX_Pipe5_Address);    
    // Define RX Address for Pipe5 (1 byte => MSBytes will be equal to RX_ADDR_P1) 
   Write_nRF24L01_Register (RX_PW_P0, RX_P0_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe0 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (RX_PW_P1, RX_P1_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe1 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (RX_PW_P2, RX_P2_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe2 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (RX_PW_P3, RX_P3_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe3 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (RX_PW_P4, RX_P4_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe4 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (RX_PW_P5, RX_P5_Payload_Width+Enable_Checksum);  
    // Define Payload Width for Pipe5 in RX Mode (extra byte if Checksum is Enabled) 
   Write_nRF24L01_Register (CONFIG, EN_CRC + CRC_Setup*0b100 + PWR_UP + PRIM_RX); 
    // nRF24L01 Configuration: RX Mode, Power Up and 2_byte CRC Encoding 
   __delay_ms(2);   // Retardo de 2 ms, seguro para cumplir con 1.5 ms mínimo

}

void Start_TX_Mode_nRF24L01 (   unsigned char TX_RX_Address_Width,  unsigned char Frequency_Channel,  unsigned char RF_Data_Rate,    unsigned char RF_Output_Power,  unsigned char LNA_Gain,  unsigned char CRC_Setup,  unsigned char Auto_Retransmit_Delay,   unsigned char Max_Auto_Retransmit, unsigned char Enable_Checksum,   unsigned char TX_RX_Payload_Width )
{
  CE=0;
  Reset_nRF24L01_Status_and_nRF24L01_Payloads();     
   // Reset the nRF24L01 Status Register and Flush TX and RX Payload Memories  
  Write_nRF24L01_Register (EN_AA, 0b00111111);      
   // Enable Auto Acknowledgement for all RX Pipes 
  Write_nRF24L01_Register (EN_RXADDR, 0b00111111);     
   // Enable all RX Pipes   
  Write_nRF24L01_Register (SETUP_AW, TX_RX_Address_Width);    
   // Set Width for all Addresses: 0b01 => 3 bytes, 0b10 => 4 bytes, 0b11 => 5 bytes  
 Write_nRF24L01_Register (SETUP_RETR, Auto_Retransmit_Delay*0b10000 + 
 Max_Auto_Retransmit);  // Automatic Retransmission Setup: Delay between and  Maximum number  
  Write_nRF24L01_Register (RF_CH, Frequency_Channel);     
   // Set the frequency channel nRF24L01 operates on  
 Write_nRF24L01_Register (RF_SETUP, RF_Data_Rate*0b1000 + RF_Output_Power*0b10 + 
 LNA_Gain);  // Set nRF24L01 RF Data Rate, Power and LNA Gain 
  Write_nRF24L01_Register (RX_PW_P0, TX_RX_Payload_Width+Enable_Checksum);  
   // Define Payload Width for Pipe0 (extra byte if Checksum is Enabled) 
  Write_nRF24L01_Register (CONFIG,EN_CRC + CRC_Setup*0b100 + PWR_UP + 0*PRIM_RX); 
   // nRF24L01 Configuration: TX Mode, Power Up and 2_byte CRC Encoding 
  __delay_ms(2);   // Retardo de 2 ms, seguro para cumplir con 1.5 ms mínimo 
} 

unsigned char Receive_Data_RX_Mode_nRF24L01( unsigned char Enable_Checksum,  unsigned char Max_Waiting_ds,   unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload )
{
    unsigned char nRF24L01_Status; 
    unsigned char Origin_Pipe; 
    unsigned char Checksum_Conclusion, IRQ_Error; 
    unsigned int i; 
    unsigned char j;
    IRQ_Error=0; 
    CE=1;    // Activate CE to leave Standby I mode and enter RX Mode (nRF24L01 searchs incoming signal) 
    __delay_us(130);  // Exacto y claro
    __delay_us(10);  // Asegura el mínimo requerido 

    i=0; j=0; 
    while(IRQ)   // Wait until RX_DR is set High (Reception of Data),and then IRQ is set Low. 
    {    // Sometimes the nRF24L01 fails to do this, so a  maximum waiting time is needed  
     i++;            
     if(i==3333)   // i reaches 3333 after 100000us = 100ms 
     {    // IRQ is analized every 30us 
      i=0; j++; 
     } 
     if(j==Max_Waiting_ds) // TIMING CONDITION #5: When we reach the Max_Waiting_ds defined by user,  loop is then broken.    
     {     // If the device did not receive data before  Max_Waiting_ds, IRQ ERROR is considered.  
      IRQ_Error=1;  // The Result of the TX operation was: IRQ ERROR 
      break;           
     } 
    } 
    CE=0;   // Deactivate CE to leave RX Mode (Low Current Consumtion) 
    __delay_us(4);  // Cumple el mínimo 
    nRF24L01_Status=Read_nRF24L01_Status();  
       // Update the nRF24L01 Status to get the Origin_Pipe 
    Origin_Pipe=(nRF24L01_Status & RX_P_NO);  
       // Data Pipe Number is obtained from the STATUS Register 
    Checksum_Conclusion=Read_nRF24L01_RX_Payload(Enable_Checksum,TX_RX_Payload_Width, RX_Payload);  // After Reading the RX FIFO, all data is deleted from FIFO.  
    Write_nRF24L01_Status(RX_DR);   
        // Clear the RX_DR bit from the nRF24L01_Status Register 
    return (IRQ_Error*0b10000+Origin_Pipe+Checksum_Conclusion);    
        // IRQ_Error, Checksum_Result and Data Pipe Number will be returned: from 0b000 to 0b101 (6 RX Pipes) 
   }

void Send_Data_TX_Mode_nRF24L01(  unsigned char Enable_Checksum,  unsigned char TX_RX_Address_Width,  unsigned char *TX_Address,  unsigned char TX_RX_Payload_Width,  unsigned char *TX_Payload ) 
{ 
    Write_nRF24L01_Address_Register (TX_RX_Address_Width, TX_ADDR, TX_Address);  
      // Define TX Address (3, 4 or 5 bytes) 
    Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P0, TX_Address); 
      // Define RX Address for Pipe0: equal to TX Address to receive Auto  Acknowledgement (3, 4 or 5 bytes) 
    Write_nRF24L01_TX_Payload (Enable_Checksum, TX_RX_Payload_Width, TX_Payload); 
      // Load Data into nRF24L01 TX FIFO, it will be sent after CE is set High 

    CE = 1;                 // Activar CE para entrar en modo RX

    __delay_us(130);        // Esperar 130 us entre Standby y RX
    __delay_us(10);         // Esperar mínimo 10 us con CE en alto

    CE = 0;                 // Desactivar CE para salir de modo RX

    __delay_us(4);          // Esperar mínimo 4 us antes de bajar CSN
    
}

unsigned char Check_Data_Sent_TX_Mode_nRF24L01 (void) 
      // Funtion to obtain TX Operation Result 
{ 
    unsigned char TX_Operation_Result, TX_Retransmit_Counter; 
    unsigned char nRF24L01_Status; 
    unsigned int i; 
    unsigned char j; 

    i=0; j=0; 
    while(IRQ)
    {
        i++; 
        if(i==333)
        {
           i=0; j++; 
        }
        if(j==7)
        {
            TX_Operation_Result=0b11;
            break;
        }
    }
    while(!IRQ)   // Process only continues if the proper bit, TX_DS or MAX_RT, was set low.  
    { 
     nRF24L01_Status=Read_nRF24L01_Status();      
      // Update the nRF24L01 Status to get cause of the IRQ 
     if(nRF24L01_Status & MAX_RT)  // If MAX_RT was set high 
     { 
        TX_Operation_Result=0b00;       
         // 0b00 => Data was not correctly sent to RX Device 
        TX_Retransmit_Counter=Read_nRF24L01_Register(OBSERVE_TX) & 0x0F;  
         // Obtain the ARC_CNT: Retransmission Counter reset after every   new packet transmission  
        Write_nRF24L01_Status(MAX_RT);   
         // We clear the MAX_RT bit from the nRF24L01_Status  Register writing a 1.  
     } 
     else if(nRF24L01_Status & TX_DS)  // If TX_DS was set high 
     { 
      TX_Operation_Result=0b01;       
       // 0b01 => Data was correctly sent to RX Device 
      TX_Retransmit_Counter=Read_nRF24L01_Register(OBSERVE_TX) & 0x0F; 
       // Obtain the ARC_CNT: Retransmission Counter reset after every  new packet transmission  
      Write_nRF24L01_Status(TX_DS);       
       // We clear the TX_DS bit from the nRF24L01_Status  Register writing a 1. 
     } 
    } 
    return (TX_Retransmit_Counter*0b100 + TX_Operation_Result);
}

void Finish_nRF24L01_Operation (void) 
{ 
    unsigned char previous_config_register;
    previous_config_register=Read_nRF24L01_Register(CONFIG);
    Write_nRF24L01_Register(CONFIG, previous_config_register & 0b11111101);
}

void Finish_SPI_Operation (void) 
{ 
    SSPCON1bits.SSPEN=0; 
} 