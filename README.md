# esp8266-pomiar-temperatury-i-wilgotnosci
Program przeznaczony dla układu esp8266 służący do pomiaru temperatury.

## Zależności / Dependencies
* https://github.com/markruys/arduino-DHT/

## Gotowe:
* obsługa czujnika: DHT22

## W przyszłości (do rozważenia):
* obsługa czujnika: DS18B20
* nodemcu / WiFi, możliwość odpytywania po API
* zapis danych w odpowiednich intwerwałach czasowych na kartę SD
* określanie
    * minimum
    * maksimum
    * odchylenia standardowego
    * średniej temperatury
        * w trakcie dnia
        * w trakcie nocy
        * w trakcie 24h
