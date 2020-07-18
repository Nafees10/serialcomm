/*
  serialcomm.cpp - Arduino library for sending data over Serial.
  Created by Nafees Hassan, July 2020.
  Released under MIT License, see LICENSE.md
*/

#include "Arduino.h"
#include "serialcomm.h"

#define HS_RQ_CONNECT 0xF0
#define HS_RP_CONNECT 0xF1
#define RQ_VAR 0xF2
#define RP_VAR 0xF3

#define TIMEOUT_MSECS 1000

/// actually registers a variable
void SerialComm::_registerVar(void* ptr, uint8_t len){
	(this->_vars[_varsUsedCount]).ptr = (uint8_t*)ptr;
	this->_vars[_varsUsedCount].length = len;
	this->_varsUsedCount ++;
}
/// Constructor
SerialComm::SerialComm(uint8_t varCount, uint8_t rxPin, uint8_t txPin, bool inverse_logic){
	_serial = new SoftwareSerial(rxPin, txPin, inverse_logic);
	_serial->setTimeout(TIMEOUT_MSECS);
	_varsCount = varCount;
	_varsUsedCount = 0;
	_vars = new VarStore [_varsCount];
}
/// registers a variable
/// 
/// Returns: ID if successful, varCount if failed
uint8_t SerialComm::registerVar(void* var, uint8_t size){
	if (this->_varsUsedCount == this->_varsCount)
		return this->_varsCount;
	uint8_t r = this->_varsUsedCount;
	_registerVar(var, size);
	return r;
}
/// establish connection
void SerialComm::connect(){
	// keep sending connection requests till it replies
	do{
		_serial->write(HS_RQ_CONNECT);
		delay(TIMEOUT_MSECS);
	}while (!_serial->available());
	// now wait for HS_RP_CONNECT before starting
	uint8_t rcv;
	bool cpConnectSent = false;
	do{
		if (_serial->available()){
			rcv = _serial->read();
			if (rcv == HS_RQ_CONNECT && !cpConnectSent){
				_serial->write(HS_RP_CONNECT);
				cpConnectSent = true;
			}
		}
	}while (rcv != HS_RP_CONNECT);
}
/// Sends a single variable, using the variable ID
void SerialComm::send(uint8_t id){
	if (id < _varsCount){
		_serial->write(RP_VAR);
		_serial->write(id);
		_serial->write(_vars[id].length);
		_serial->write((const uint8_t*)_vars[id].ptr, _vars[id].length);
	}
}
/// sends all registered variables
void SerialComm::send(){
	for (uint8_t i = 0; i < _varsCount; i ++){
		_serial->write(RP_VAR);
		_serial->write(i);
		_serial->write(_vars[i].length);
		_serial->write((const uint8_t*)_vars[i].ptr, _vars[i].length);
	}
}
/// waits till timeout for commands, or new values, responds if necessary, & 
/// updates local values
void SerialComm::update(uint8_t timeout){
	unsigned long t = millis();
	while (!_serial->available()){
		if (millis() - t >= timeout)
			break;
	}
	while (_serial->available()){
		uint8_t command, id, length, *buffer;
		command = _serial->read();
		if (command == RQ_VAR){
			// read the id
			while (!_serial->available()){}
			id = _serial->read();
			this->send(id);
		}else if (command == RP_VAR){
			// store this var
			while (!_serial->available()){}
			id = _serial->read();
			while (!_serial->available()){}
			length = _serial->read();
			buffer = new uint8_t[length];
			_serial->readBytes(buffer, length);
		}
	}
}
/// use this instead of regular delay() when timeout is > 100msecs
void SerialComm::delay(uint8_t timeout){
	unsigned long t = millis();
	update(timeout);
	if (millis() - t < timeout){
		unsigned long delayPeriod = timeout - (millis() - t);
		delay(delayPeriod > timeout ? 0 : delayPeriod);
	}
}