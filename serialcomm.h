/*
  serialcomm.cpp - Arduino library for sending data over Serial.
  Created by Nafees Hassan, July 2020.
  Released under MIT License, see LICENSE.md
*/

#ifndef serialcomm_h
#define serialcomm_h

#include "Arduino.h"
#include "SoftwareSerial.h"

/// Manages sending and receiving of data over SoftwareSerial.
/// Can handle upto 241 variables. Do not register more than that
class SerialComm{
private:
	struct VarStore{
		uint8_t* ptr;
		uint8_t length;
	};
	SoftwareSerial *_serial;
	VarStore *_vars;
	uint8_t _varsCount;
	uint8_t _varsUsedCount;
	void _registerVar(void* ptr, uint8_t len);
public:
	SerialComm(uint8_t varCount, uint8_t rxPin, uint8_t txPin, bool inverse_logic=false);
	/// registers a variable
	/// 
	/// Returns: ID if successful, varCount if failed
	uint8_t registerVar(void* var, uint8_t size);
	/// establish connection
	void connect();
	/// Sends a single variable, using the variable ID
	void send(uint8_t id);
	/// sends all registered variables
	void send();
	/// waits till timeout for commands, or new values, responds if necessary, & 
	/// updates local values
	void update(uint8_t timeout);
	/// use this instead of regular delay() when timeout is > 100msecs
	void delay(uint8_t timeout);
};

#endif