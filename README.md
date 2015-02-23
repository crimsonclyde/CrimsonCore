<pre>
 _________        .__
 \_   ___ \_______|__| _____   __________   ____
 /    \  \/\_  __ \  |/     \ /  ___/  _ \ /    \
 \     \____|  | \/  |  | |  \\___ (  <_> )   |  \
  \______  /|__|  |__|__|_|  /____  >____/|___|  /
        \/                \/     \/  Core      \/   </pre>



**Author  :** CrimsonClyde<br>
**Email   :** crimsonclyde_at_gmail.com<br>
**Version :** Beta 7.5<br>
**Codename:** Michael F. Stoppe<br>


### Project Discription:
First of all the project is dedicated to one of my most beloved adventures ManiacMansion from 1987. The introducion of the digital Skywalker Ranch which is also used in the sequel Day of the Tentacle.

* beta 1 = Jeff Woodie        "Surfer Dude"
* beta 2 = Sandy Pantz        "Stereotypical blonde cheerleader"
* beta 3 = Ted                "Dead cousin"
* beta 4 = Dr. Fred Edision   "Mad scientist"
* beta 5 = Razor              "Punk Girl"
* beta 6 = Syd                "Guy who put the hamster in the microwave"
* beta 7 = Michael F. Stoppe  "Award winning photographer"

### So, whats that heck is that all about?
Fun and learning, not more not less. I start by lighting up a simple LED with an Arduino Nano. A few days later I started to build twilight control to light up 230V LEDs to illuminate my stone circle. Days go by and I find myself into building up a my own home automation control. I learned quite a lot and after switching to a Spark Core (https://spark.io) I added a temperature sensor and store the results in a database. Now I am able to use a bunch of program languages and scripting skills. So never stop learning!

### Why I use Names from the game Maniac Mansion?
I liked the game and still the complete genre. A good project needs a good codename so I used this.


### Roadmap

#### Beta 5 "Razor" 
Razor is a punk girl. The lead-singer of the band "Razor and the Scummetts" This is punk because it is completely rewritten. Due to the Architecture change from Arduino (Atmel ATMega328) to Spark Core (STM32F103CB - ARM 32-bit Cortex M3). Not all what I have done with the Arduino runs exactly on the Spark Core. Example: I do not need a RTC Module anymore, because I am connected to Wifi and therefore I can use time syncronisation (just like NTP). Beta 5 should be the proof-of-concept if everything works and then I can impliment new features like webaccess.

#### Beta 6 "Syd"
The only one who accepts to put the hamster in the microwave. So we have nearly all done. Things will work. Only a few of them not. OLED Display runs a button to turn it off or on is installed. The OLED-Icons are upgreaded. The function to save energy is installed know as deep-sleep-mode. CrimsonCTRL works now pretty smoth. Some recode needed to get it to HTML5 with change the Mobile/Web view only over the stylesheet. The buttons did not work for switching the StoneCircleLight directly over the webinterface.

#### Beta 7 "Michael F. Stoppe"
Introducing the first half-backed-nearly-ready codebase and the web buttons works now. We can
switch the Stone-Circle-Light on/off over our HTML5 page. A bunch of bugfixes and workarounds. E.g. there was a problem when the Core has no internet/cloud connection in this case no code is executed. The Core tries over and over again to get the connection back on, in this time no code is executed. The GPIO pin used for the Stone-Circle-Light is set to LOW which means the relay is closed and therefore the light is on. This is bad because if we are not at home and the internet is down we waste power for nothing. With late beta 6 and now stable in beta 7 we switched to Semi-Automated mode. If this is set, the Core first execute the code. Setup() runs pin is set to HIGH, light is off we get into the loop() and now we can connect on our on purpose.


### Conclusion:
How knows, maybe someday I find the fuel for the chainsaw in another game.

### Ressources:
The libraries used in this project are not written nor owned by myself. Rights and Kudos goes all to the people who made this work. If an Author is missing, and you are the one, or know the Author please drop me a email.

- DigoleSerialDisplay OLED : https://github.com/timothybrown/Spark-Core-Sundries
- OneWire Library          : https://github.com/krvarma/Dallas_DS18B20_SparkCore
- DS18B20 Library          : https://github.com/krvarma/Dallas_DS18B20_SparkCore
- Adam Dunkels Protothreads: https://code.google.com/p/arduinode/downloads/detail?name=pt.zip
- Bitmaps Creator          : http://pixieengine.com/pixel-editor
- Bitmaps Converter        : http://www.digole.com/tools/PicturetoC_Hex_converter.php

### Wiring:
* A1  -   Photo Resistor -        - grey
* A2  -   Digole SS               -
* A3  -   Digole CLK              -
* A5  -   Digole Data             -
* GND -   5V- (Bell)              - black
* VIN -   5V+ (Bell)              - purple
* D2  -   Temp Sensor Data        - yellow
* D3  -   SCL Relais Data         - white
* GND -   Photo Resistor +        - red


### Licence (without any of the libraries)
Copyright (C) 2015 CrimsonClyde

These programs are free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

Third party libraries are used under their own terms and are NOT involved by this copyright. All of my code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You can receive a copy of the GNU Affero General Public License on: <http://www.gnu.org/licenses/agpl.html>.
