# MAX7219-Dot-Matrix-90-Degree-Demo
A simple demo using an Arduino and MAX7219 display modules, used are the modules you can find cheap on eBay. These modules have the matrix display mounted at 90 degrees to what most library's expect.

Yet to do is to complete this project as a C++ library, I just haven't had the time yet, but it’s on my list.

#Notes
The demo was written for an Arduino but also works with a ESP8266. The only change is the GPIO connections and the SPI speed, see below.

  SPI.setFrequency(15000000L);    // Only needed for ESP8266, does not work with UNO

David
