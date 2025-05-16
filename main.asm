    LIST P=18F4550
    #include <P18F4550.INC>
    
    ORG 0X1000
    goto INICIO
    
    


    
INICIO
    clrf    PORTA
    clrf    PORTD
    clrf    LATA
    clrf    LATD
    ;SE LIMPIAN LOS REGISTROS
    bsf TRISA, 0 ; SE HABILITA A0 COMO ENTRADA
    ; Configurar ADCON1: AN0 analógico, demás digitales
    movlw   b'00001110'
    movwf   ADCON1
    ; Configurar ADCON2: Right justified, Tacq = 8TAD, Fosc/32
    movlw   b'101010'          ; 0b00101010
    movwf   ADCON2
    ; Configurar ADCON0: Canal AN0, ADC encendido
    movlw   b'00000001'
    movwf   ADCON0
    
    
Get_ADC:
    ; Iniciar la conversión A/D
    bsf     ADCON0, 1      ; Set ADCON0, bit 1 (GO/DONE) para iniciar la conversión

Wait_ADC:
    ; Esperar a que la conversión A/D termine
    btfsc   ADCON0, 1      ; Verificar si la conversión ha terminado (GO/DONE = 0)
    goto    Wait_ADC       ; Si no ha terminado, seguir esperando
    
    ; Para obtener los 8 bits más significativos (escalado a 8 bits):
    movf     ADRESH, W         
    ;movwf    ADC_8bit_Value
    END