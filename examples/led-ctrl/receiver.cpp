#include <Arduino.h>
#include "serialcomm.h"
#include "SoftwareSerial.h"

#define LEDPIN 13
#define RX 2
#define TX 3

SoftwareSerial swSerial(RX, TX);
SerialComm<SoftwareSerial, 3> net(&swSerial);
bool ledStatus;

void setup() {
	pinMode(LEDPIN, OUTPUT);
	swSerial.begin(9600);
	net.registerVar(&ledStatus, sizeof(ledStatus));
	net.awaitConnection(); // wait for other arduino to be on before going further
}

void loop() {
	digitalWrite(LEDPIN, ledStatus ? HIGH : LOW);
	net.sleep(250);
}
