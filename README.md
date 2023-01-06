# Прошивка для COOLRF HeatStick на базе ESPHome

## COOLRF HeatStick

COOLRF HeatStick - модуль управления различными климатическими устройствами 
(конвекторами, водонагревателями, кондиционерами) ряда производителей (Ballu, Zanussi, Electrolux).

Устанавливается вместо штатного модуля управления производителя. Заменяет:
- Ballu Smart Wi-Fi BEC/WF-01
- Zanussi ZCH/WF-01 Smart Wi-Fi
- Electrolux Smart Wi-Fi ECH/WF-01
- Ballu Smart Wi-Fi BEC/WFN-02 (требуется проверка)

В отличие от оригинального модуля позволяет интегрировать управление климатическим устройством 
в вашу систему умного дома на базе Home Assistant, либо в любую другую DIY-систему.

### Поддерживаемые устройства

- Конвектор Ballu Evolution Transformer с инверторным блоком управления.

### Потенциально поддерживаемые устройств

Устройства из этого раздела либо сразу заработают с модулем. Либо заработают после коррекции прошивки.

- Водонагреватель Ballu SMART WiFi
- Водонагреватель ZANUSSI SPLENDORE XP 2.0
- Водонагреватель Electrolux MEGAPOLIS WiFi
- Конвектор Ballu Apollo Transformer с инверторным блоком управления
- Конвектор Electrolux Transformer System с инверторным блоком управления
- Кондиционер Ballu Platinum Evolution DC inverter
- Кондиционер Ballu Lagoon DC inverter 2021

### Подробнее о проекте

- https://habr.com/ru/company/coolrf/blog/
  - https://habr.com/ru/company/coolrf/blog/589381/ HeatStick рулит. Конвектором Ballu
- https://vk.com/coolrf
- https://www.instagram.com/coolrf/

### Где купить?

- https://www.avito.ru/ekaterinburg/tovary_dlya_kompyutera/coolrf_heatstick_upravlenie_konvektoromboylerom_2663725969

## Сборка прошивки
```
sudo pip install esphome
mkdir ~/coolrf
cd ~/coolrf
git clone https://github.com/coolrf/heatstick-esphome.git
nano wifi.yaml # содержимое файла указано ниже
cd heatstick-esphome
esphome run heatstick-esphome.yaml
```
### Требования к версиям

ESPHome 2021.11.2 или выше.
Python 3 или выше.

### Файл wifi.yaml

Файл с настройками паролей к беспроводным сетям вынесен за пределы репозитория. Для сборки прошивки его необходимо создать вручную.

```yaml

wifi:
  networks:
  - ssid: "One"
    password: "Password"
  - ssid: "Two"
    password: "Password"
  ap: {}

ota:
```
