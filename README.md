# Biblioteca nRF24L01 para PIC18F4550 y configuracion para modo transmisor manda blink a otro modulo


basado en el proyecto de Sanz Fernández, Pablo Comunicación por RF entre microcontroladores PIC18 mediante el módulo NRF24L01  https://upcommons.upc.edu/handle/2099.1/23530





Este repositorio contiene una implementación en lenguaje C para controlar el módulo de radiofrecuencia **nRF24L01** utilizando el protocolo SPI en un microcontrolador **PIC18F4550**.

## ✅ Características

- Comunicación SPI con el módulo nRF24L01.
- Funciones para lectura y escritura de registros del nRF24L01.
- Basado en registros de bajo nivel del PIC18F4550.
- Código ligero y fácil de adaptar a otros proyectos embebidos.

## 🛠️ Requisitos

- **Microcontrolador:** PIC18F4550  
- **Compilador:** MPLAB XC8 versión 1.41  (no he probado con otros compiladores)
- **Entorno de desarrollo:** MPLAB X IDE (recomendado)
- **consideraciones:** 

## ⚡ Pines utilizados

| Señal       | Pin PIC18F4550 | Descripción          |
|-------------|----------------|----------------------|
| `SDI` (MISO) | RB0            | Entrada de datos SPI |
| `SCK`        | RB1            | Reloj SPI            |
| `SDO` (MOSI) | RC7            | Salida de datos SPI  |
| `CSN`        | A3   	        | Chip Select nRF24L01 |
| `CE`         | A2             | Chip Enable nRF24L01 |


---

Si usas este código en tus proyectos, ¡no olvides dar crédito!

Modificacion  por **Luis Fernando** 🚀  Autor original **Sanz Fernández**
¡Gracias por apoyar este repositorio!

![Reaver](https://media.tenor.com/zZFdvazBM_YAAAAM/reaver-starcraft.gif)