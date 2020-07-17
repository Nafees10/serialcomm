#include "Arduino.h"

#ifndef USE_HARDWARE_SERIAL
 	#include <SoftwareSerial.h>
#endif

#define HS_RQ_CONNECT 0xF0
#define HS_RP_CONNECT 0xF1
#define RQ_VAR 0xF2
#define RP_VAR 0xF3
#define RQ_ARR 0xF4
#define RP_ARR 0xF5
#define RQ_PING 0xF6
#define RP_PING 0xF7

#define TIMEOUT_MSECS 1000

struct VarStore{
	void* ptr;
	uint8_t length;
};

/// Manages sending and receiving of data over HardwareSerial or SoftwareSerial.
/// Can handle upto 241 variables. Do not register more than that
class SerialComm{
private:
	#if defined(USE_HARDWARE_SERIAL)
		HardwareSerial *_serial;
	#else
		SoftwareSerial *_serial;
	#endif
	VarStore *_vars;
	uint8_t _varsCount;
	uint8_t _varsUsedCount;
	/// actually registers a variable
	void _registerVar(void* ptr, uint8_t len){
		_vars[_varsUsedCount].ptr = ptr;
		_vars[_varsUsedCount].length = len;
		_varsUsedCount ++;
	}
public:
	#ifdef USE_HARDWARE_SERIAL
		SerialComm(HardwareSerial &serial, uint8_t varCount)){
			_serial = serial;
			_varsCount = varCount;
			_varsUsedCount = 0;
			_vars = new VarStore[_varsCount];
		}
	#else
		SerialComm(SoftwareSerial &serial, uint8_t varCount){
			_serial = serial;
			_varsCount = varCount;
			_varsUsedCount = 0;
			_vars = new VarStore[_varsCount];
		}
	#endif
	/// registers a variable
	/// 
	/// Returns: ID if successful, varCount if failed
	template <class T>
	unsigned char registerVar(T &var){
		if (_varsUsedCount == _varsCount)
			return _varsCount;
		uint8_t r = _varsUsedCount;
		_registerVar(var, sizeof(T));
		return r;
	}
	/// establish connection
	void connect(){
		// keep sending connection requests till it replies
		do{
			_serial.write(HS_RQ_CONNECT);
			delay(TIMEOUT_MSECS);
		}while (!_serial.available());
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
			
		}
	}
};