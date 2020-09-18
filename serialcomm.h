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
#include "Stream.h"

#define HS_RQ_PING 0xF0
#define HS_RP_PING 0xF1
#define RQ_VAR 0xF2
#define RP_VAR 0xF3

/// Manages sending and receiving of data over Serial.
/// Can handle upto 128 variables, plus 112 strings
template <uint8_t VarCount>
class SerialComm{
	struct VarStore{
		/// pointer to where the variable lives
		uint8_t* ptr;
		/// number of bytes in the variable.
		uint8_t length;
	};
	Stream *_serial;
	VarStore _vars[VarCount];
	uint8_t _varsUsedCount;
	unsigned long _maxLatency;
	unsigned long _lastPing;
	void _registerVar(uint8_t* ptr, uint8_t len){
		(_vars[_varsUsedCount]).ptr = ptr;
		_vars[_varsUsedCount].length = len;
		_varsUsedCount ++;
	}
public:
	/// Constructor
	SerialComm(Stream *sr){
		_serial = sr;
		_varsUsedCount = 0;
		_maxLatency = 1000;
	}
	/// Sets maximum latency for ping timeouts. Default is 1000msecs
	void setMaxLatency(unsigned long latency){
		_maxLatency = latency;
	}
	/// registers a variable. This works for:
	/// 1. Regular varialbes (int, float, char ...)
	/// 2. Static arrays
	/// Do not use this with:
	/// 1. strings (you can use a fixed-length array of char instead)
	/// 2. Dynamic arrays (because pointers)
	/// 
	/// Returns: ID if successful, 255 if failed
	uint8_t registerVar(void* var, uint8_t size){
		if (this->_varsUsedCount >= VarCount)
			return 255;
		uint8_t r = this->_varsUsedCount;
		_registerVar((uint8_t*)var, size);
		return r;
	}
	/// Sends a ping, waits for reply. Use this to check if connected or not.
	/// Returns: true if ping reply arrived, false if 
	/// Writes latency (milliseconds) to `latency`
	bool ping(unsigned long &latency){
		// check if some data already received
		if (_serial->available())
			this->update(0);
		_serial->write(HS_RQ_PING);
		latency = millis();
		while (millis() - latency < _maxLatency){
			if (_serial->available()){
				latency = millis() - latency;
				if (_serial->read() == HS_RP_PING)
					return true;
				return false;
			}
		}
		latency = millis() - latency;
		return false;
	}
	/// Sends a ping. waits for reply. Returns true if reply arrived in time, false if not
	bool ping(){
		unsigned long latency;
		return ping(latency);
	}
	/// Sends a single variable, using the variable ID
	void send(uint8_t id){
		if (id < VarCount){
			_serial->write(RP_VAR);
			_serial->write(id);
			_serial->write(_vars[id].length);
			_serial->write(_vars[id].ptr, _vars[id].length);
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
	/// waits till timeout for commands, or new values, responds if necessary, & 
	/// updates local values.
	/// Set timeout to zero to wait indefinitely for update
	/// Returns: true if some data was sent or received, false if nothing was done (might indicate a connection lost)
	void update(uint8_t timeout = 0){
		unsigned long t = millis();
		/// wait for data to arrive on serial, with timeout
		while (_serial->available() == 0){
			if (timeout > 0 && millis() - t >= timeout)
				break;
		}
		while (_serial->available()){
			uint8_t command, id, length, *buffer;
			command = _serial->read();
			if (command == HS_RQ_PING){
				// send RP_PING back
				_serial->write(HS_RP_PING);
			}else if (command == RQ_VAR){
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
				while (_serial->available() == 0){}
				if (id >= VarCount || length != _vars[id].length){
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
	void sleep(unsigned long timeout){
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
