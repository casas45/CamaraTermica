/*
 * ATmega328p_SPItoAnalog.c
 *
 * Created: 01/10/2022 01:38:06 p. m.
 * Author : Luis David Casas H
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

/*Se crean definiciones para pines MISO, MOSI, SCK, SS_SENSOR, SS_DAC*/
#define MOSI 3
#define MISO 4
#define SCK 5
#define CS_sensor 2
#define CS_DAC 1

#define DAC 0x3000	//valor para habilitar bits SHDN y GA en el MCP4921

/*Declaración de funciones*/
void SPI_init (void);
void transmitSPIbyte(uint8_t dataT);
uint8_t receiveSPIbyte(void);

int main(void)
{
	/*Se habilita la comunicación SPI*/
	SPI_init();
	
	/*Variables para almacenar y procesar los datos*/
	uint8_t HighByte = 0;
	uint8_t LowByte = 0;
	uint8_t HighByteDAC = 0;
	uint8_t LowByteDAC = 0;
	uint16_t data = 0;

	while (1)
	{
		/*Lectura del sensor*/
		PORTB &= ~(1<<CS_sensor);	
		_delay_us(1);
		HighByte = receiveSPIbyte();	//se recibe el pimer byte	(MSByte)
		LowByte = receiveSPIbyte();	//se recibe el segundo byte (LSByte)
		PORTB |= (1<<CS_sensor);
		_delay_us(1);
		
		/*En LowByte los primeros 3 bits no aportan información
		y se hace una operación OR para obtener los 12 bits de la temperatura en data */
		data = (HighByte << 5) | (LowByte >> 3);
		data |= DAC;	//se utiliza DAC para habilitar los bits necesarios
		
		/*Se divide data en dos bytes*/
		HighByteDAC = data>>8;
		LowByteDAC = data&0xFF;
		
		_delay_us(50);
		
		/*Envio de datos hacia el MCP4921 (DAC)*/
		PORTB &= ~(1<<CS_DAC);
		_delay_us(1);
		transmitSPIbyte(HighByteDAC);
		transmitSPIbyte(LowByteDAC);
		PORTB |= (1<<CS_DAC);
		_delay_us(1);
		
		_delay_ms(50);
	}
}

void SPI_init(void){
	DDRB = (1<<MOSI) | (1<<SCK) |  (1<<CS_sensor) | (1<<CS_DAC) | (0<<MISO);		//se habilitan las salidas necesarias en el puerto B
	SPCR = (1<<SPE) | (0<<DORD) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA) | (0<<SPR0) | (0<<SPR1);	//se habilita comunicación SPI como maestro y SCK = CLK/2 = 4MHz
	SPSR = (1<<SPI2X);
	PORTB |= (1<<CS_sensor) | (1<<CS_DAC); //Se inicializan CS_sensor y CS_DAC en estado alto
}

void transmitSPIbyte(uint8_t dataT){
	//Se carga el byte al registro SPDR
	SPDR = dataT;
	//Se espera hasta que se haga el envio
	while (!(SPSR & (1 << SPIF)));
}

uint8_t receiveSPIbyte(void){
	//Se manda un byte fiticio para luego recibir datos
	SPDR = 0xFF;
	//Se espera a que se haga la recepción
	while (!(SPSR & (1 << SPIF)));
	//Se retorna el valor del registro SPDR
	return SPDR;
}