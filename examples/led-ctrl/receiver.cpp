#include <Arduino.h>
#include "serialcomm.h"

#define LEDPIN 13

SerialComm<HardwareSerial, 1> net(&Serial);
bool ledStatus;

void setup() {
	pinMode(LEDPIN, OUTPUT);
	Serial.begin(9600);
	net.registerVar(&ledStatus, sizeof(ledStatus));
	net.awaitConnection(); // wait for other arduino to be on before going further
}

void loop() {
	digitalWrite(LEDPIN, ledStatus ? HIGH : LOW);
	net.sleep(250);
}
