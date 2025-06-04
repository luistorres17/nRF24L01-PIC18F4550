#include <xc.h>
#include <stdint.h>
#include "nRF24l01.h"
#include "config.h"





//configuracion funciona con el NRF24L01 Y EL PIC , COMPROBAR DATASHEET PIC18F4550 PARA ENTENDIMIENTO.
void SPI_Start (unsigned char Clock_Frequency)  
{  
    TRISBbits.RB1 = 0;      //RB1 -> SCK
    TRISBbits.RB0 = 1;      //RB0 -> SDI(MISO)
    TRISCbits.RC7 = 0;      //RC7 -> SDO(MOSI)
    SSPCON1 = (SSPCON1 & 0xF0) | Clock_Frequency; //Configura la velocidad del reloj SPI usando el par�metro Clock_Frequency. es la velocidad con la que se comunicara preferentemente usar FOSC/16 
    //para el pic 18f4550
    SSPCON1bits.CKP=0;  
    SSPSTATbits.CKE=1;   
    SSPSTATbits.SMP=1;  
    PIE1bits.SSPIE=0;   
    IPR1bits.SSPIP=0;   
    PIR1bits.SSPIF=0;  
    SSPCON1bits.SSPEN=1;     
}

void nRF24L01_Ports_Start(void) 
{ 
    ADCON1=0x0F;  // SE HABILITAN COMO DIGITALES TODOS LOS PUERTOS DE AN
    TRISAbits.TRISA2=0; // CE SE CONFIGURA COMO SALIDA
    TRISAbits.TRISA3=0;  // CSN SE CONFIGURA COMO SALIDA
    TRISBbits.TRISB2=1;  // INTERRUPCION DEL NRFL2401 SE CONFIGURA EN EL PIC COMO ENTRADA
    CSN=1;  //para iniciar el programa csn se debe configurar como alto es decir se encontrara inctivo
    CE=0;  // para iniciar el programa ce se debe configurar como bajo y de igual manera quiere decir que se encontrara inactivo. 
}   


unsigned char SPI_Transfer(unsigned char byte_to_send)   
{ 
    SSPBUF = byte_to_send;  //el dato a enviar se carga en el buffer ssp, cuando el modulo se encuentre listo se intercambia por otro entrante. 

    while(PIR1bits.SSPIF==0); // espera hasta que este se cambie
    PIR1bits.SSPIF=0;  // al cambiar se limpia la bandera de interrupcion del spi

    return(SSPBUF);  // retornamos el byte recibido
}

unsigned char Read_nRF24L01_Register (unsigned char Register_Address)  
{ 
    unsigned char Register_Content, nRF24L01_Status; 

    CSN=0;   
    nRF24L01_Status=SPI_Transfer(R_REGISTER+Register_Address);   
       
    Register_Content=SPI_Transfer(0x00);  
        
    CSN=1;    
    return(Register_Content); 
} 

void Write_nRF24L01_Register (unsigned char Register_Address,unsigned char Register_Content )
{
    unsigned char dummydata, nRF24L01_Status;
    CSN=0;   
    nRF24L01_Status=SPI_Transfer(W_REGISTER+Register_Address);   
     
    dummydata=SPI_Transfer(Register_Content);      
    
    CSN=1;  
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

    CSN=0;     
    nRF24L01_Status=SPI_Transfer(NOP);   
    CSN=1;     

    return(nRF24L01_Status);   
} 

void Write_nRF24L01_Status (unsigned char Register_Content)      
{ 
    unsigned char dummydata, nRF24L01_Status; 

    CSN=0;    
    nRF24L01_Status=SPI_Transfer(W_REGISTER+STATUS);       
         
    dummydata=SPI_Transfer(Register_Content);      
         
    CSN=1;   
} 

void Read_nRF24L01_Address_Register (  unsigned char TX_RX_Address_Width,  unsigned char Register_Address,  unsigned char *Register_Content )  
{ 
    unsigned char i, nRF24L01_Status; 

    CSN=0;   
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
    
    Process_Finished=0;      
    while(!Process_Finished)
    {
        CSN=0;   
        nRF24L01_Status=SPI_Transfer(W_REGISTER+Register_Address);
        for(i=0; i<(TX_RX_Address_Width+0b10); i++)
        {
            dummydata=SPI_Transfer(Register_Content[i]);
        }
        CSN=1;   
 
        Read_nRF24L01_Address_Register(TX_RX_Address_Width, Register_Address, Address_Verification); 
        Process_Finished=1;    
        for(i=0; i<(TX_RX_Address_Width+0b10); i++)
        {
            if(Register_Content[i]!=Address_Verification[i])    
                { 
                 Process_Finished=0;  
                  
                } 
        }
    }
}

 
unsigned char Read_nRF24L01_RX_Payload ( unsigned char Enable_Checksum, unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload  ) 
{
    unsigned char nRF24L01_Status, TX_Checksum;      
   
    unsigned char RX_Checksum, Checksum_Conclusion;     
   
    unsigned char i;         
        
    CSN=0;  
    nRF24L01_Status=SPI_Transfer(R_RX_PAYLOAD);  
    
    if(!Enable_Checksum) 
    { 
        for(i=0; i<TX_RX_Payload_Width; i++)  
        { 
         RX_Payload[i]=SPI_Transfer(0x00);     
             
        } 
        Checksum_Conclusion=1;  
    }
    else if(Enable_Checksum) 
    { 
        RX_Checksum=0b00000000; 
        for(i=0; i<TX_RX_Payload_Width; i++) 
        { 
            RX_Payload[i]=SPI_Transfer(0x00);     
                 
            RX_Checksum=RX_Checksum+RX_Payload[i];    
                
        } 
        TX_Checksum=SPI_Transfer(0x00); 
        if(RX_Checksum==TX_Checksum) 
        { 
            Checksum_Conclusion=1;  
        } 
        else if(RX_Checksum!=TX_Checksum) 
        { 
            Checksum_Conclusion=0;   
        } 
    } 
    CSN=1;  
 
    return Checksum_Conclusion;
}

void Write_nRF24L01_TX_Payload (  unsigned char Enable_Checksum,  unsigned char TX_RX_Payload_Width,  unsigned char *TX_Payload  )  
{ 
    unsigned char nRF24L01_Status, TX_Checksum; 
    unsigned char i, dummydata; 

    CSN=0;      
    nRF24L01_Status=SPI_Transfer(W_TX_PAYLOAD); 
    if(!Enable_Checksum)    
    { 
        for(i=0; i<TX_RX_Payload_Width; i++) 
        { 
            dummydata=SPI_Transfer(TX_Payload[i]); 
             
        } 
    }  
    else if(Enable_Checksum) 
    { 
        TX_Checksum=0b00000000; 
        for(i=0; i<TX_RX_Payload_Width; i++)  
        { 
            dummydata=SPI_Transfer(TX_Payload[i]); 
             
            TX_Checksum=TX_Checksum+TX_Payload[i];    
                
        }  
        dummydata=SPI_Transfer(TX_Checksum);      
        
    } 
    CSN=1;    
}

void Reset_nRF24L01_Status_and_nRF24L01_Payloads (void)    
{ 
    unsigned char nRF24L01_Status; 

    CSN=0;      
    nRF24L01_Status=SPI_Transfer(FLUSH_TX); // Send Instruction to Flush TX while  receiving nRF24L01 Status 
    CSN=1;      
    __delay_us(1);  
    CSN=0;        
    nRF24L01_Status=SPI_Transfer(FLUSH_RX); // Send Instruction to Flush RX while receiving nRF24L01 Status
    CSN=1;       
 
    Write_nRF24L01_Status(RX_DR);    
    Write_nRF24L01_Status(TX_DS);   
    Write_nRF24L01_Status(MAX_RT);  
} 

void Start_RX_Mode_nRF24L01 (   unsigned char TX_RX_Address_Width,  unsigned char Frequency_Channel,  unsigned char RF_Data_Rate,    unsigned char RF_Output_Power,  unsigned char LNA_Gain,  unsigned char CRC_Setup,  unsigned char *RX_Pipe0_Address,   unsigned char *RX_Pipe1_Address, unsigned char RX_Pipe2_Address,   unsigned char RX_Pipe3_Address,  unsigned char RX_Pipe4_Address, unsigned char RX_Pipe5_Address,  unsigned char Enable_Checksum, unsigned char RX_P0_Payload_Width, unsigned char RX_P1_Payload_Width,  unsigned char RX_P2_Payload_Width, unsigned char RX_P3_Payload_Width,  unsigned char RX_P4_Payload_Width, unsigned char RX_P5_Payload_Width ) 
{
   CE=0;    
   Reset_nRF24L01_Status_and_nRF24L01_Payloads();     
    
   Write_nRF24L01_Register (EN_AA, 0b00111111);      
    
   Write_nRF24L01_Register (EN_RXADDR, 0b00111111);     
    
   Write_nRF24L01_Register (SETUP_AW, TX_RX_Address_Width);    
    
   Write_nRF24L01_Register (RF_CH, Frequency_Channel);     
    
  Write_nRF24L01_Register (RF_SETUP, RF_Data_Rate*0b1000 + RF_Output_Power*0b10 + LNA_Gain); 
   Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P0, RX_Pipe0_Address);
   
   Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P1, RX_Pipe1_Address);
    
   Write_nRF24L01_Register (RX_ADDR_P2, RX_Pipe2_Address);    
     
   Write_nRF24L01_Register (RX_ADDR_P3, RX_Pipe3_Address);    
     
   Write_nRF24L01_Register (RX_ADDR_P4, RX_Pipe4_Address);    
     
   Write_nRF24L01_Register (RX_ADDR_P5, RX_Pipe5_Address);    
     
   Write_nRF24L01_Register (RX_PW_P0, RX_P0_Payload_Width+Enable_Checksum);  
    
   Write_nRF24L01_Register (RX_PW_P1, RX_P1_Payload_Width+Enable_Checksum);  
    
   Write_nRF24L01_Register (RX_PW_P2, RX_P2_Payload_Width+Enable_Checksum);  
    
   Write_nRF24L01_Register (RX_PW_P3, RX_P3_Payload_Width+Enable_Checksum);  
     
   Write_nRF24L01_Register (RX_PW_P4, RX_P4_Payload_Width+Enable_Checksum);  
     
   Write_nRF24L01_Register (RX_PW_P5, RX_P5_Payload_Width+Enable_Checksum);  
   
   Write_nRF24L01_Register (CONFIG, EN_CRC + CRC_Setup*0b100 + PWR_UP + PRIM_RX); 
    
   __delay_ms(2);   

}

void Start_TX_Mode_nRF24L01 (   unsigned char TX_RX_Address_Width,  unsigned char Frequency_Channel,  unsigned char RF_Data_Rate,    unsigned char RF_Output_Power,  unsigned char LNA_Gain,  unsigned char CRC_Setup,  unsigned char Auto_Retransmit_Delay,   unsigned char Max_Auto_Retransmit, unsigned char Enable_Checksum,   unsigned char TX_RX_Payload_Width )
{
  CE=0;
  Reset_nRF24L01_Status_and_nRF24L01_Payloads();     
    
  Write_nRF24L01_Register (EN_AA, 0b00111111);      
    
  Write_nRF24L01_Register (EN_RXADDR, 0b00111111);     
   
  Write_nRF24L01_Register (SETUP_AW, TX_RX_Address_Width);    
   
 Write_nRF24L01_Register (SETUP_RETR, Auto_Retransmit_Delay*0b10000 + 
 Max_Auto_Retransmit);   
  Write_nRF24L01_Register (RF_CH, Frequency_Channel);     
    
 Write_nRF24L01_Register (RF_SETUP, RF_Data_Rate*0b1000 + RF_Output_Power*0b10 + LNA_Gain);  
 Write_nRF24L01_Register (RX_PW_P0, TX_RX_Payload_Width+Enable_Checksum);  
   
  Write_nRF24L01_Register (CONFIG,EN_CRC + CRC_Setup*0b100 + PWR_UP + 0*PRIM_RX); 
  __delay_ms(2);    
} 

unsigned char Receive_Data_RX_Mode_nRF24L01( unsigned char Enable_Checksum,  unsigned char Max_Waiting_ds,   unsigned char TX_RX_Payload_Width,  unsigned char *RX_Payload )
{
    unsigned char nRF24L01_Status; 
    unsigned char Origin_Pipe; 
    unsigned char Checksum_Conclusion, IRQ_Error; 
    unsigned int i; 
    unsigned char j;
    IRQ_Error=0; 
    CE=1;    
    __delay_us(130);  
    __delay_us(10);  // Asegura el m�nimo requerido 

    i=0; j=0; 
    while(IRQ)   
    {     
     i++;            
     if(i==3333)   
     {    
      i=0; j++; 
     } 
     if(j==Max_Waiting_ds)   
     {      
      IRQ_Error=1;  
      break;           
     } 
    } 
    CE=0;    
    __delay_us(4);   
    nRF24L01_Status=Read_nRF24L01_Status();  
       
    Origin_Pipe=(nRF24L01_Status & RX_P_NO);  
       
    Checksum_Conclusion=Read_nRF24L01_RX_Payload(Enable_Checksum,TX_RX_Payload_Width, RX_Payload);
    Write_nRF24L01_Status(RX_DR);   
        
    return (IRQ_Error*0b10000+Origin_Pipe+Checksum_Conclusion);    
         
   }

void Send_Data_TX_Mode_nRF24L01(  unsigned char Enable_Checksum,  unsigned char TX_RX_Address_Width,  unsigned char *TX_Address,  unsigned char TX_RX_Payload_Width,  unsigned char *TX_Payload ) 
{ 
    Write_nRF24L01_Address_Register (TX_RX_Address_Width, TX_ADDR, TX_Address);  
       
    Write_nRF24L01_Address_Register (TX_RX_Address_Width, RX_ADDR_P0, TX_Address); 
      
    Write_nRF24L01_TX_Payload (Enable_Checksum, TX_RX_Payload_Width, TX_Payload); 
      

    CE = 1;                 

    __delay_us(130);        
    __delay_us(10);         

    CE = 0;                 

    __delay_us(4);          
    
}

unsigned char Check_Data_Sent_TX_Mode_nRF24L01 (void) 
       
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
    while(!IRQ)     
    { 
     nRF24L01_Status=Read_nRF24L01_Status();      
       
     if(nRF24L01_Status & MAX_RT)  
     { 
        TX_Operation_Result=0b00;       
          
        TX_Retransmit_Counter=Read_nRF24L01_Register(OBSERVE_TX) & 0x0F;  
           
        Write_nRF24L01_Status(MAX_RT);   
           
     } 
     else if(nRF24L01_Status & TX_DS)  // If TX_DS was set high 
     { 
      TX_Operation_Result=0b01;       
       
      TX_Retransmit_Counter=Read_nRF24L01_Register(OBSERVE_TX) & 0x0F; 
        
      Write_nRF24L01_Status(TX_DS);       
        
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