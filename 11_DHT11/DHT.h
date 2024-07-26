#ifndef DHT_H_INCLUDED
#define DHT_H_INCLUDED
/*
||
||  Filename:	 		DHT.h
||  Title: 			    DHTxx Driver
||  Author: 			Efthymios Koktsidis
||	Email:				efthymios.ks@gmail.com
||  Compiler:		 	AVR-GCC
||	Description:
||	This library can drive DHT11 and DHT22 sensors.
||
*/
//------ Headers ------//
#define F_CPU	16000000UL
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h> 
#include "IO_Macros.h"
#include "DHT_Settings.h"
//----------------------//

//----- Auxiliary data -------------------//
#define DHT11						 1
#define DHT22						 2

#define __DHT_DELAY_SETUP			2000

enum DHT_Status_t
{
	DHT_Ok,
	DHT_Error_Humidity,
	DHT_Error_Temperature,
	DHT_Error_Checksum,
	DHT_Error_Timeout
};
//-----------------------------------------//

//----- Prototypes---------------------------//
void DHT_Setup(void);
enum DHT_Status_t DHT_status(void);
void DHT_ReadRaw(uint8_t Data[4]);
void DHT_ReadTemperature(uint8_t *Temperature);
void DHT_ReadHumidity(uint8_t *Humidity);
void DHT_Read(uint8_t *Temperature, uint8_t *Humidity);
double DHT_ConvertToFahrenheit(double Temperature);
double DHT_ConvertToKelvin(double Temperature);
//-------------------------------------------//
#endif