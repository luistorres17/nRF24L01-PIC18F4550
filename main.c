#include <xc.h>
#include <stdint.h>
#include "config.h"
#include "nRF24l01.h"

void main(void)
{
    unsigned char direccion_tx[5] = {0xA1, 0xA1, 0xA1, 0xA1, 0xA1};
    unsigned char mensaje[1];
    unsigned char estado = 0;

    ADCON0bits.ADON = 0;

    SPI_Start(0b01);
    nRF24L01_Ports_Start();
    Start_TX_Mode_nRF24L01(0b11, 0x01, 0, 0b11, 1, 1, 0x00, 10, 0, 1);

    while (1)
    {
        mensaje[0] = estado;  // Enviar 0x00 o 0x01
        Send_Data_TX_Mode_nRF24L01(0, 0b11, direccion_tx, 1, mensaje);
        Check_Data_Sent_TX_Mode_nRF24L01();

        estado ^= 0x01; // Alterna entre 0 y 1

        __delay_ms(500);
    }
}
