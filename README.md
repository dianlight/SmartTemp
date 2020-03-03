<!--
*** Thanks for checking out this README Template. If you have a suggestion that would
*** make this better, please fork the repo and create a pull request or simply open
*** an issue with the tag "enhancement".
*** Thanks again! Now go create something AMAZING! :D
***
***
***
*** To avoid retyping too much info. Do a search and replace for the following:
*** github_username, repo, twitter_handle, email
-->





<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]




<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/dianlight/SmartTemp">
    <!--
    <img src="images/logo.png" alt="Logo" width="80" height="80">
    -->
  </a>

  <h3 align="center">Smart Temp</h3>

  <p align="center">
    An DIY Smart Thermostat
    <br />
    <!--
    <a href="https://github.com/dianlight/SmartTemp"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    -->
    <a href="https://github.com/dianlight/SmartTemp/blob/master/CHANGELOG.md">Changelog</a>
    ·
    <a href="https://github.com/dianlight/SmartTemp/issues">SmartTemp Bug</a>
    ·
    <a href="https://github.com/dianlight/SmartTemp/issues">Request Feature</a>
  </p>
</p>

<!-- Donations -->

<a href="https://www.buymeacoffee.com/ypKZ2I0" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" style="height: 51px !important;width: 217px !important;" ></a>


<!-- TABLE OF CONTENTS -->
## Table of Contents

- [Table of Contents](#table-of-contents)
- [About The Project](#about-the-project)
  - [Built With](#built-with)
- [Getting Started](#getting-started)
  - [Hardware Prerequisites](#hardware-prerequisites)
  - [Circuit](#circuit)
  - [Environment Prerequisites](#environment-prerequisites)
  - [Installation](#installation)
- [Config.h options](#configh-options)
- [Features / Whishlist](#features--whishlist)
- [Roadmap](#roadmap)
- [User Manual](#user-manual)
  - [Encoder function](#encoder-function)
  - [Thermal confort mode (Advanced)](#thermal-confort-mode-advanced)
  - [PMV/PPD method](#pmvppd-method)
  - [Modes](#modes)
  - [In house / away selection](#in-house--away-selection)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)
- [Acknowledgements](#acknowledgements)



<!-- ABOUT THE PROJECT -->
## About The Project

<!-- [![Product Name Screen Shot][product-screenshot]](https://example.com) -->
Just another smart thermostat!

In a smart home, a smart thermostat cannot be missing. So why not build one at low cost and with all the features that interest me.

The project started using material left over from other projects and respecting the following requirements:

* Low cost
* Independence from Cloud services (privacy first!)
* Small size (must be contained in a US size box)
* Be smart but if necessary work without WiFi and / or external controller
* KISS. Being able to be used by my wife, daughter, son, mother, father, grandmother, grandfather -> the whole family!

### Built With

* [Platform IO](http://platformio.org)


<!-- GETTING STARTED -->
## Getting Started

To get a local copy up and running follow these simple steps.

### Hardware Prerequisites

For a detailed list of components refer to the documentation (BOM)

1. I2C Advance Extender based on ATMEGA8A-PU (See project [at8i2cgateway](https://github.com/dianlight/a8i2cGateway))
2. MCU ESP-01 or ESP-01S
3. Serial programmer
4. I2C Oled display
5. Rotary encoder
6. DHT22 sensor
7. Relay 
8. 5v power source ( I used an Hi-Link 220v to 5v but you can use alse batteries )
9. Wires and Connectors 

### Circuit

***!Work in progress!***

### Environment Prerequisites

1. Platform IO ( to compile and generate the firmware )
2. An HomeAssistant installation ( used as Smart Controller ) ___(Optional)___ 
3. An MQTT Borker ___(Optional)___
4. An WiFi capable device like SmartPhone.
5. An WiFi internet connection

### Installation
 
1. Burn the bootloader into the ATMEGA8 with Arduino IDE and the SPI/ASP programmer
2. Clone the SmartTemp
```sh
git clone https://github.com/dianlight/SmartTemp.git
```
3. Open the project with Platform IO
4. Configure the firmware 
```src/include/Config.h```
5. Build and flash your firmware


<!-- USAGE EXAMPLES -->
## Config.h options

***!Work in progress!***

<!-- FEATURES -->
## Features / Whishlist

- [x] Standalone Thermostat
  - [x] Current Room Temperature
  - [x] Current Room Humidity
  - [x] 3 levels of temperatures. (ECO, NORMAL and CONFORT)
  - [x] Week programming (15min resolution)
- [ ] Supported units
  - [x] Celsius mode
  - [ ] Fahrenheit mode ( [Wikipedia](https://en.wikipedia.org/wiki/Fahrenheit) )
- [x] Advanced Thermostat
  - [x] Away/In_house temp control
  - [x] Auto, Manual and Off mode
  - [ ] Thermal confort mode 
    - [ ] [PMV/PPD method](https://en.wikipedia.org/wiki/Thermal_comfort#PMV/PPD_method)
    - [ ] [Adaptive comfort model](https://en.wikipedia.org/wiki/Thermal_comfort#Adaptive_comfort_model)<sup>1</sup>
    - [ ] [ASHRAE-55](https://comfort.cbe.berkeley.edu/)<sup>2</sup>
    - [ ] [EN-16798](https://comfort.cbe.berkeley.edu/)<sup>2</sup>
- [x] Smart Thermostat (Wifi remote controlled)
  - [x] MQTT transport
  - [x] HomeAssistant climate compatibility (via MQTT integration)
    - [ ] Homeassistant room presence integration ( auto AWAY/NORMAL mode )
  - [x] NTP clock sync (daylight auto switch)
    - [ ] Time Zone configuration
- [x] WebInterface
  - [ ] Mobile WebApp
- [ ] External sensor
  - [ ] Wifi temperature sensor
  - [ ] Wifi pressure sensor
  - [ ] Wifi air speed sensor
  - [ ] Home Assistant collector
- [x] Enclosure (3D STL printable models)
  - [x] Italian BTicino Living 2Gang
  - [ ] EU standard
  - [ ] UK standard
  - [ ] US standard

_*Checked are implemented_

_<sup>1</sup>Require external temperature sensor_

_<sup>2</sup>Require external temperature, pressure sensor and internet connection_



<!-- ROADMAP -->
## Roadmap

See the [open issues](https://github.com/dianlight/SmartTemp/issues) for a list of proposed features (and known issues).

<!-- USER MANUAL -->
## User Manual

### Encoder function

***!Work in progress!***

### Thermal confort mode (Advanced)

***!Work in progress not yet implemented!*** 

The thermal confort mode allows you to set the thermostat not based on the real  desired temperature but on the basis of the [confort temperature](https://en.wikipedia.org/wiki/Thermal_comfort#Interplay_of_temperature_and_humidity). This significantly increases comfort by reducing consumption.

### [PMV/PPD method](https://en.wikipedia.org/wiki/Thermal_comfort#PMV/)

![Temp/Humidity](https://upload.wikimedia.org/wikipedia/commons/thumb/a/a0/Temperature-relative_humidity_chart_-_PMV_method.png/1024px-Temperature-relative_humidity_chart_-_PMV_method.png)


### Modes 

| Mode | Description | Home modfity | Away modify | Note |
| --- | --- | --- | --- | --- |
| Auto | Follow the Week program to set *Eco*, *Normal* or *Confort* target temp | none | &plusmn; delta | |
| Manual | Follow the setting of selected target temp | none | &plusmn; delta | return to *Auto* mode after impoisted time |

### In house / away selection

| Selection | Mode | Description | Function | Note |
| --- | --- | --- | --- | --- |
| In house  | Auto | Standard mode | Follow week program | |
| In house  | Manual | Temporary mode | Follow selected target temperature until timeout expire | Automatic return to *Auto* mode |
| Away      | Auto | Advanced mode | Follow week program with a &plusmn; temperature delta | |
| Away      | Eco  | Advanced mode | Lock to *Eco* target temperature  with a &plusmn;  delta | No automatic return to *Auto* mode |
| Away      | Normal  | Advanced mode | Lock to *Normal* target temperature with a &plusmn;  delta | No automatic return to *Auto* mode |
| Away      | Confort  | Advanced mode | Lock to *Confort* target temperature with a &plusmn;  delta | No automatic return to *Auto* mode |



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request



<!-- LICENSE -->
## License

Distributed under the GPL3 License. See `LICENSE` for more information.



<!-- CONTACT -->
## Contact

<!--
Your Name - [@twitter_handle](https://twitter.com/twitter_handle) - email
-->

Project Link: [https://github.com/dianlight/SmartTemp](https://github.com/dianlight/SmartTemp)



<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements

Icons made by <a href="https://www.flaticon.com/authors/pixel-perfect" title="Pixel perfect">Pixel perfect</a> from <a href="https://www.flaticon.com/" title="Flaticon"> www.flaticon.com</a>

<!--
* [RobTillaart](https://github.com/RobTillaart/) for the Arduino libraries
* [soligen2010](https://github.com/soligen2010/) for the ClickEncoder fork
  -->



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/dianlight/SmartTemp.svg?style=flat-square
[contributors-url]: https://github.com/dianlight/SmartTemp/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/dianlight/SmartTemp.svg?style=flat-square
[forks-url]: https://github.com/dianlight/SmartTemp/network/members
[stars-shield]: https://img.shields.io/github/stars/dianlight/SmartTemp.svg?style=flat-square
[stars-url]: https://github.com/dianlight/SmartTemp/stargazers
[issues-shield]: https://img.shields.io/github/issues/dianlight/SmartTemp.svg?style=flat-square
[issues-url]: https://github.com/dianlight/SmartTemp/issues
[license-shield]: https://img.shields.io/github/license/dianlight/SmartTemp.svg?style=flat-square
[license-url]: https://github.com/dianlight/SmartTemp/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=flat-square&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/lucio-tarantino-8ab9a3/
[product-screenshot]: images/screenshot.png
[buy-me-a-coffe]: https://www.buymeacoffee.com/ypKZ2I0
