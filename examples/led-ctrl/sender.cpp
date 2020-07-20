#include <Arduino.h>
#include "serialcomm.h"
#include "SoftwareSerial.h"

#define BUTTONPIN 4
#define RX 2
#define TX 3

SoftwareSerial swSerial(RX, TX);
SerialComm<SoftwareSerial, 3> net(&swSerial);
bool ledStatus;

void setup() {
	pinMode(BUTTONPIN, INPUT_PULLUP);
	swSerial.begin(9600);
	net.registerVar(&ledStatus, sizeof(ledStatus), true); // true sets it to auto-send every time net.sleep or net.update called
	net.awaitConnection(); // wait for other arduino to be on before going further
}

void loop() {
	ledStatus = digitalRead(BUTTONPIN) == HIGH ? true : false;
	net.sleep(250);
}
