/*
  serialcomm.cpp - Arduino library for sending data over Serial.
  Created by Nafees Hassan, July 2020.
  Released under MIT License, see LICENSE.md
*/

#ifndef serialcomm_h
#define serialcomm_h

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "HardwareSerial.h"

#define HS_RQ_CONNECT 0xF0
#define HS_RP_CONNECT 0xF1
#define RQ_VAR 0xF2
#define RP_VAR 0xF3

#define TIMEOUT_MSECS 1000

/// Manages sending and receiving of data over SoftwareSerial.
/// Can handle upto 241 variables. Do not register more than that
template <class T>
class SerialComm{
	struct VarStore{
		uint8_t* ptr;
		uint8_t length;
	};
	T *_serial;
	VarStore *_vars;
	uint8_t _varsCount;
	uint8_t _varsUsedCount;
	void _registerVar(void* ptr, uint8_t len){
		(_vars[_varsUsedCount]).ptr = (uint8_t*)ptr;
		_vars[_varsUsedCount].length = len;
		_varsUsedCount ++;
	}
public:
	/// Constructor
	SerialComm(T *sr, uint8_t varCount){
		_serial = sr;
		_varsCount = varCount;
		_varsUsedCount = 0;
		_vars = new VarStore [_varsCount];
	}
	/// registers a variable
	/// 
	/// Returns: ID if successful, varCount if failed
	uint8_t registerVar(void* var, uint8_t size){
		if (this->_varsUsedCount == this->_varsCount)
			return this->_varsCount;
		uint8_t r = this->_varsUsedCount;
		_registerVar(var, size);
		return r;
	}
	/// establish connection
	void connect(){
		// keep sending connection requests till it replies
		do{
			_serial->write(HS_RQ_CONNECT);
			delay(TIMEOUT_MSECS);
		}while (_serial->available() == 0);
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
	void send(uint8_t id){
		if (id < _varsCount){
			_serial->write(RP_VAR);
			_serial->write(id);
			_serial->write(_vars[id].length);
			_serial->write((const uint8_t*)_vars[id].ptr, _vars[id].length);
		}
	}
	/// sends all registered variables
	void send(){
		for (uint8_t i = 0; i < _varsCount; i ++){
			_serial->write(RP_VAR);
			_serial->write(i);
			_serial->write(_vars[i].length);
			_serial->write((const uint8_t*)_vars[i].ptr, _vars[i].length);
		}
	}
	/// waits till timeout for commands, or new values, responds if necessary, & 
	/// updates local values
	void update(uint8_t timeout){
		unsigned long t = millis();
		while (_serial->available() == 0){
			if (millis() - t >= timeout)
				break;
		}
		while (_serial->available()){
			uint8_t command, id, length, *buffer;
			command = _serial->read();
			if (command == RQ_VAR){
				// read the id
				while (_serial->available() == 0){}
				id = _serial->read();
				this->send(id);
			}else if (command == RP_VAR){
				// store this var
				while (_serial->available() == 0){}
				id = _serial->read();
				while (_serial->available() == 0){}
				length = _serial->read();
				buffer = new uint8_t[length];
				_serial->readBytes(buffer, length);
			}
		}
	}
	/// use this instead of regular delay() when timeout is > 100msecs
	void sleep(uint8_t timeout){
		unsigned long t = millis();
		update(timeout);
		if (millis() - t < timeout){
			unsigned long delayPeriod = timeout - (millis() - t);
			delay(delayPeriod > timeout ? 0 : delayPeriod);
		}
	}
};

#undef HS_RQ_CONNECT
#undef HS_RP_CONNECT
#undef RQ_VAR
#undef RP_VAR

#endif