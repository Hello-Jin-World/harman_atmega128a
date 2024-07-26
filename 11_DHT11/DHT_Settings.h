#ifndef DHT_SETTINGS_H_INCLUDED	
#define DHT_SETTINGS_H_INCLUDED
/*
||
||  Filename:	 		DHT_Settings.h
||  Title: 			    DHTxx Driver Settings
||  Author: 			Efthymios Koktsidis
||	Email:				efthymios.ks@gmail.com
||  Compiler:		 	AVR-GCC
||	Description:
||	Settings for the DHTxx driver. Pick a model 
||	and the desirable pin.
||
*/

//----- Configuration --------------------------//
#define DHT_Type	DHT11          //DHT11 or DHT22
// #define DHT_Pin		C, 0       // Orginal
#define DHT_Pin		G, 0
//----------------------------------------------//
#endif