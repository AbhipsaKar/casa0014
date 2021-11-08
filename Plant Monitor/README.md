Overview of the Plant monitor setup:
Components used from IOT kit
1. Feather huzzah ESP8266 microcontroller
2. Breadboard
3. Jumper cables
4. DHT(Temperature,Humidity sensor)- DHT 22
5. Transistor (BC 547)
6. USB cable(For power)
7. Resistors(2 * 10 K Ohms, 1 * 220 Ohms, 2 * 100 Ohms)
8. Header pins for custom made circuit
9. 2 Nails(to act as the soil sensor)

Creation of the temporary circuit:
To simplify the circuit diagram , I have separated the whole circuit into small circuits, as displayed below:
![image](https://user-images.githubusercontent.com/91799774/139703560-d11bd215-ad9d-43b0-a199-812c96f56a10.png)

I found the following diagrams(from CE HUB workshop) helpful to understand the pin out arrangement of the Huzzaf feather Mcu:
![image](https://user-images.githubusercontent.com/91799774/140049082-d39b7693-9da2-4d65-8bb1-f506cf930b95.png)
![image](https://user-images.githubusercontent.com/91799774/140049219-26946cb2-e0a8-4664-9d3c-b70f07ed51d6.png)

The below image shows the picture of the completed circuit.
![image](https://user-images.githubusercontent.com/91799774/139704129-c027a4cb-fa80-4c6e-b515-3f085e66f4dd.png)

NOTE: 2 nails act as the soil moisture sensors by measuring the amount of voltage passed between the nails. To setup the sensor, Wind one end of a wire around the head of the nail tightly to enable conductivity and the other end is plugged to the breadboard. The nails are then inserted into the plant to take the readings.
The below image shows the setup: 
![image](https://user-images.githubusercontent.com/91799774/139704235-12630a30-94b5-405c-b80d-cb3ec444b7ae.png)

Running the sketch on the circuit:
Once the circuit is connected on the breadboard, plug the microcontroller to the computer via USB and open the Arduino APP.

Install the below libraries from the 'Manage libraries' setting screen.
1. DHT sensor libraries
2. ESP MQTT client
3. ezTime
4. MQTT pubsubclient
5. ESP Wifi
6. ESP webserver

Upload the new binary to the micro-controller and open the command line to check the sensor readings.
The arduino sketch pushes the sensor data to the MQTT server.
The below image shows the side by side view of the command line and the MQTT server.
![image](https://user-images.githubusercontent.com/91799774/139705567-a8c61036-5113-44c2-9575-daa653b5eea1.png)

Final setup:
Once we can got the temporary circuit running correctly with the breadboard, we can create a soldered circuit with all the components and plug it on top of the Huzzah feather to create the final setup. Below are the images of the same.
![image](https://user-images.githubusercontent.com/91799774/139705861-0cfc3ac4-554c-4fd0-837f-b568718a2fce.png)
![image](https://user-images.githubusercontent.com/91799774/139705962-cee03fee-9bb6-4bf2-8a91-4399ec4efecd.png)

Additionally, the sensor readings are also sent across Http to a webpage for the server. The same can be viewed on the IP address created by the ESP WIFI module. A sample screen can be observed below:
![image](https://user-images.githubusercontent.com/91799774/140059924-6b4079ed-cda7-4cc0-9ce3-7d2a6e4ae6fc.png)

Personalise the setup:
The plant I have chosen is the 'Christmas cactus'. This belongs to the succulent family of plants and retain water.
I found the below plant instructions on the web(Source: https://www.crocus.co.uk/plants/_/schlumbergera-pink-purple/classid.2000043302/?affiliate=googleproductfeed&gclid=Cj0KCQjwwNWKBhDAARIsAJ8HkheRKBqu9I6hv5GpXiMUXPw5ncwgBsbANdLl2zx12Ashj_9RMeCLqywaAmeGEALw_wcB) for this particular plant:

1. Light: A Christmas cactus should be kept in a bright place, with indirect light.
2. Water: Water your Christmas cactus only when the soil gets dry to the touch. But during the flowering season, it is crucial to keep the soil evenly moist by misting it frequently.
3. Temperature: The optimal temperature for your Christmas cactus is between 15˚C to 21˚C (60˚F to 70˚F), with average to high levels of humidity.
4. Fertilizer: Once buds have formed, fertilize your Christmas cactus with high-potassium fertilizer every two weeks once buds form.

I felt that instead of fixing the sensor reading interval to 1 sec, it could be changed depending on weather conditions. Hence , I have added a variable to set the interval in the arduino sketch. The ESP client name and MQTT topic name have also been updated based on my username.

Moving to Raspberry Pi:
Once the Raspberry imager is flashed with the latest binary, you can configure a user name and password which can be used to login into SSH and control the Raspberry Pi.

After successfully logging in to Raspberry Pi, install and configure Influx DB, Telegraf and Grafana. 
Influx DB: is a time series DB which is very ideal for storing data.
Telegraf:  is the open source server agent to help you collect metrics from your stacks, sensors and systems.
Grafana: is the dashboard to fetch and visualise the data from DB.

1. We need to confirgure the HTTP endpoint for influx DB by updating the following variables in influxdb.conf:
[http]
  # Determines whether HTTP endpoint is enabled.
  enabled = true

  # The bind address used by the HTTP service.
  bind-address = ":8086"

  # Determines whether user authentication is enabled over HTTP/HTTPS.
  auth-enabled = true

2. I have attached the configuration file of telegraf inside the github repository.
3. Start the grafana server on SSH using command: sudo systemctl start telegraf.service and go to website: http://stud-pi-ucfnaka:3000/d/ZwS7ZPZRk/dashboard-plantmonitor?orgId=1&from=now-24h&to=now
Host name: stud-pi-ucfnaka
Port num: 3000

The below screenshot is a sample image of the grafana dashboard where I have added the influxDB as a datasource and queried records from mqtt.consumer table.
![image](https://user-images.githubusercontent.com/91799774/140770478-9711ab32-ab02-4fe0-90a3-5f1ce3b7dd2e.png)

I have create one combined dashboard for all 3 attributes and a separate dashboard for each attribute.
With this, we have completed the plant monitor setup.

Future ideas:
1. Adding a photo-diode to measure the intensity of light.
2. Adding an reminder for fertilizing the plant every 2 weeks.
3. Adding mechanical parts to the setup to make it self-watering.
4. Get the sensor reading interval from the web interface/MQTT topic as input.
