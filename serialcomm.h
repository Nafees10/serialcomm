/*
  serialcomm.h - Arduino library for sending data over Serial.
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

#define HS_RQ_DELAY 1000

/// Manages sending and receiving of data over Serial.
/// Can handle upto 128 variables, plus 112 strings
template <class T, uint8_t VarCount>
class SerialComm{
	struct VarStore{
		/// pointer to where the variable lives
		uint8_t* ptr;
		/// number of bytes in the variable.
		uint8_t length;
	};
	T *_serial;
	VarStore _vars[VarCount];
	uint8_t _varsUsedCount;
	bool _varsAutoSend[VarCount];
	void _registerVar(uint8_t* ptr, uint8_t len){
		(_vars[_varsUsedCount]).ptr = ptr;
		_vars[_varsUsedCount].length = len;
		_varsUsedCount ++;
	}
public:
	/// Constructor
	SerialComm(T *sr){
		_serial = sr;
		_varsUsedCount = 0;
		for (uint8_t i = 0; i < VarCount; i ++)
			_varsAutoSend[i] = false;
	}
	/// registers a variable. This works for:
	/// 1. Regular varialbes (int, float, char ...)
	/// 2. Static arrays
	/// Do not use this with:
	/// 1. strings (you can use a fixed-length array of char instead)
	/// 2. Dynamic arrays (because pointers)
	/// 
	/// Returns: ID if successful, 255 if failed
	uint8_t registerVar(void* var, uint8_t size, bool autoSend = false){
		if (this->_varsUsedCount >= VarCount)
			return 255;
		uint8_t r = this->_varsUsedCount;
		_registerVar((uint8_t*)var, size);
		_varsAutoSend[r] = autoSend;
		return r;
	}
	/// waits for other device to respond.
	/// Call this in setup() if you want to wait for both devices to be online,
	/// or if you want them to sync up.
	void awaitConnection(){
		unsigned long t = millis();
		// keep sending connection requests till it replies
		do{
			_serial->write(HS_RQ_CONNECT);
			while (_serial->available() == 0 && millis() - t < HS_RQ_DELAY){}
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
		if (id < VarCount){
			_serial->write(RP_VAR);
			_serial->write(id);
			_serial->write(_vars[id].length);
			_serial->write((const uint8_t*)_vars[id].ptr, _vars[id].length);
		}
	}
	/// sends all registered variables
	void send(){
		for (uint8_t i = 0; i < VarCount; i ++){
			_serial->write(RP_VAR);
			_serial->write(i);
			_serial->write(_vars[i].length);
			_serial->write((const uint8_t*)_vars[i].ptr, _vars[i].length);
		}
	}
	/// send request to send a variable (using variable ID)
	/// call update() after this to receive the value
	void request(uint8_t id){
		if (id < VarCount){
			_serial->write(RQ_VAR);
			_serial->write(id);
		}
	}
	/// waits till timeout for commands, or new values, responds if necessary, & 
	/// updates local values.
	/// Set timeout to zero to wait indefinitely for update
	void update(uint8_t timeout){
		unsigned long t = millis();
		/// send the autoSend variables
		for (uint8_t i = 0; i < VarCount; i ++){
			this->send(i);
		}
		/// wait for data to arrive on serial, with timeout
		while (_serial->available() == 0){
			if (timeout > 0 && millis() - t >= timeout)
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
				if (id >= VarCount || length > _vars[id].length){
					// read Serial just to clear it, ignore the values
					buffer = new uint8_t[length];
					_serial->readBytes(buffer, length);
					delete[] buffer;
				}else{
					_serial->readBytes(_vars[id].ptr, length);
				}
				
			}
		}
	}
	/// prefer this instead of regular delay() when timeout is big enough for update()
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
