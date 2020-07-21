#include <Arduino.h>
#include "serialcomm.h"

SerialComm<HardwareSerial, 1> net(&Serial);
bool ledStatus;

void setup() {
	pinMode(BUTTONPIN, INPUT_PULLUP);
	Serial.begin(9600);
	net.registerVar(&ledStatus, sizeof(ledStatus), true); // true sets it to auto-send every time net.sleep or net.update called
	net.awaitConnection(); // wait for other arduino to be on before going further
}

void loop() {
	ledStatus = !ledStatus;
	net.sleep(250);
}
