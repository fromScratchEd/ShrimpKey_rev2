ShrimpUSB as programmer
======================

Make a (bread)board like this one: http://arduino.cc/en/uploads/Tutorial/BreadboardAVR.png (without the Arduino).

Connect it like this:

Programmer	->	Target

D10		->	Reset
D11		->	D11
D12		->	D12
D13		->	D13
+5V		->	+5V
GND		->	GND

Upload the hex-file to the programmer with avrdude

avrdude -c usbasp -p atmega328p -u -U flash:w:rev3_usbaspprogrammer_atmega328p.hex
or
avrdude -c usbasp -p atmega8 -u -U flash:w:rev3_usbaspprogrammer_atmega8.hex

After uploading press the reset button of the Shrimp to start it in programmer mode.