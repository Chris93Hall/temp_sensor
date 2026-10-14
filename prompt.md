
I have an esp8266 ESP-12F board and a DHT22/AM2302 digital temperature and humidity sensor.
I'd like to use these two components together to monitor the temperature and humidity
in my house.

- Create a "src" directory which will contain all source code
- In the src directory, write some code for interfacing with the DHT22/AM2302 to get temperature and humidity readings

- In the src directory, create a web api interface for retreiving the temperature and humidity readings.
  - Should have an endpoint for getting the temperature and humidity as a json object. Include a timestamp in the json (both ISO format and J1970 time)
  - Another endpoint should show a clean and basic html page with the same data that the first endpoint returns
  - Another endpoint for returning the uptime (in json)

- Add a datasheets directory which will include any relevant datasheets for the esp8266, ESP-12F, and DHT22/AM2302

- Add a docs directory with documentation for the code in the src directory.

- Add a README.md which explains how to get started, compile the code (if it's written in a compiled language), and install the code on the chip
  - Also include instructions on how to connect the sensor to the board (which pins to which pins).

- add a config directory
  - In this directory, I'd like to be able to specify the IP address that my web api serves from
