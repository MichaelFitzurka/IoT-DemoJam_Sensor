# IoT-DemoJam_Sensor
Internet of Things DemoJam :: Sensor

---
Based on [PatrickSteiner/IoT_Demo_Sensors](https://github.com/PatrickSteiner/IoT_Demo_Sensors)  
Specifically: [DHTServerMQTT.ino](https://github.com/PatrickSteiner/IoT_Demo_Sensors/blob/master/DHTServerMQTT/DHTServerMQTT.ino)  
Contains additions for:
- Hosting web pages on the chip.
- Using English "on" & "off" light commands.
- Filtering for MAX_INT (2147483647) readings that can happen when the circuit is loose, and which accidentally trigger a "Temperature high" event.

If you are building this out for yourself, be sure to edit; [iotdj-sensor-code.ino](https://github.com/MichaelFitzurka/IoT-DemoJam_Sensor/blob/master/iotdj-sensor-code/iotdj-sensor-code.ino):
> Line 21: Use your own router SSID.  
> Line 22: Use your own router password.  
> Line 25: Use your own IP address for the SmartGateway (In the DemoJam projects we are are using a Raspberry Pi.)  
> Line 26: Use your own Fuse port, although "1883" is the standard.  
> Line 27: Use your own username for Fuse on the Pi/SmartGateway.  
> Line 28: Use your own password for Fuse on the Pi/SmartGateway.  
> Lines 136, 171 & 189: Change the author, if you desire. (I don't mind!)

## Notes:
In the DemoJam event/projects, we use 3 sensors with differently colored LED lights; yellow, blue and green.

The LED mirrors the light on the ESP8266 chip itself, providing more visibility to the crowd, and the colors are used to distinguish the sensors throughout the code.

In my setup, the yellow sensor is powered by batteries attached to the circuit/breadboard, in order to be completely disconnected.  The blue and green sensors are powered by a USB hub attached to the ESP8266 chips and the UPS, but are otherwise unattached.  Power at your discretion.

Having the spares really helped, as the yellow sensor stopped working during the event (**!!!**) and we were able to immediately switch to the green sensor and save the demo.

The web pages are a helpful addition for troubleshooting, turning the lights on and off manually, and are visible in the demo from the [Drone-Cam](https://github.com/MichaelFitzurka/IoT-DemoJam_Drone/tree/master/iotdj-dronecam) page.
