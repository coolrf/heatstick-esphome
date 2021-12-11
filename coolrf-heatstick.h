/* 
 * This file is part of the COOLRF HeatStick project (https://github.com/coolrf/heatstick-esphome).
 * Copyright (c) 2015 Liviu Ionescu.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "esphome.h"
#include "esphome/core/log.h"

#include <map>

const char *const log_tag = "heatstick"; // prefix for log messages


/* Data package indexes */
// 1st byte = AA
// 2ns byte is data bytes count
uint8_t const IDX_CMD	 	= 2;
uint8_t const IDX_STATUS 	= 3; // +
uint8_t const IDX_TEMP_PLAN	= 4; // +
uint8_t const IDX_MODE 		= 5; // +
uint8_t const IDX_POWER 	= 6; // +
uint8_t const IDX_TIMER_V	= 7; // 2 bytes
uint8_t const IDX_TIMER_S	= 9;
uint8_t const IDX_TEMP_REAL	= 10; // +
uint8_t const IDX_POWER_LEV	= 11; // 
// 1 byte unknown
uint8_t const IDX_DISPLAY	= 13; // +
// last byte = 8bit sum of all package bytes

uint8_t const CMD_CHANGED	 = 0x09; // device state changed (from device)
uint8_t const CMD_SET		 = 0x0A; // set device state (from heatstck)
uint8_t const CMD_SETOK		 = 0x8A; // device state ok (from device, answer to 0x0A)
uint8_t const CMD_SERVER	 = 0x88; // send data to server??


/* Parameters to bytes mappings */

std::map<byte, std::string> heatstick_status_map = {
    { 0x00, "off" },
    { 0x01, "on" },
    { 0x03, "block" }
};

std::map<byte, std::string> heatstick_mode_map = {
    { 0x01, "comfort" },
    { 0x02, "night" },
    { 0x03, "nofrost" }
};

std::map<byte, std::string> heatstick_power_map = {
    { 0x01, "lev1" },
    { 0x02, "lev2" },
    { 0x03, "lev3" },
    { 0x04, "lev4" },
    { 0x05, "lev5" },
    { 0x06, "auto" }
};

std::map<byte, std::string> heatstick_display_map = {
    { 0x00, "on" },
    { 0x01, "off" },
};
    
/*
    Current device status
    0xFF before first state
*/
std::vector<byte> heatstick_state = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*
    Get device state command
    Used for receive first device state
*/
std::vector<byte> heatstick_get_state_packet = {0xAA, 0x03, 0x08, 0x10, 0x04, 0xC9}; 


/* Global functions */

byte heatstick_find_by_value( std::map<byte, std::string> &m, std::string v ) {
   byte key = 0;
   for (auto &i : m) {
      if (i.second == v) {
         key = i.first;
         break; // stop searching
      }
   }
   return key;
}

uint8_t heatstick_checksum(std::vector<byte> &v) {
    uint8_t sum = 0x00;
    for(uint8_t i = 0; i < v.size() - 1; sum += v[i++]) {}
        ESP_LOGD(log_tag, "checksum: %u", sum); 
    return(sum);
}

std::vector<byte> heatstick_set_state_packet() {
    heatstick_state[heatstick_state.size()-1] = heatstick_checksum(heatstick_state);
    return heatstick_state;
}

void heatstick_set_state_string( byte i, std::map<byte, std::string> m, std::string v ) {
    byte d = heatstick_find_by_value(m,v);
    if( heatstick_state[i] != d ) { // чтобы избежать повторной лишней передачи данных при publish - проверить, необходимо ли
        heatstick_state[i] = d;
        if(heatstick_state[0]!=0xFF) { id(send_data).execute(); }
    }
}


/* YAML Functions */

void heatstick_set_status( std::string x ) {
    heatstick_set_state_string(IDX_STATUS, heatstick_status_map, x);
}

void heatstick_set_power( std::string x ) {
    heatstick_set_state_string(IDX_POWER, heatstick_power_map, x);
}

void heatstick_set_mode( std::string x ) {
    heatstick_set_state_string(IDX_MODE, heatstick_mode_map, x);
}

void heatstick_set_temp( byte x ) {
    if(heatstick_state[IDX_TEMP_PLAN] != x) {
        heatstick_state[IDX_TEMP_PLAN] = x;
        if(heatstick_state[0]!=0xFF) { id(send_data).execute(); }
    }
}

void heatstick_set_display( std::string x ) {
    heatstick_set_state_string(IDX_DISPLAY, heatstick_display_map, x);
}


/* UART Receiver */

class HeatStickUART : public Component, public UARTDevice {
    public:
        HeatStickUART(UARTComponent *parent) : UARTDevice(parent) {}

    std::vector<byte> msg;

    void setup() override {
        // nothing to do here
	}

    void loop() override {

        if(available()) {

            while (available()) {
              uint8_t data; 
              read_byte(&data);
              msg.push_back(data);
            }

            if(msg.size()>=4) { // если есть минимальный пакет данных
                if(msg[1]==msg.size()-3) { // если все сообщение получено
                    if(msg[msg.size()-1] == heatstick_checksum(msg)) { // если контрольная сумма верна

                        if(msg.size()==15) { // пока умеем парсить только 15-значные пакеты
                            state_diff(msg);
                        } else { ESP_LOGD(log_tag, "message length != 15"); }

                    } else { ESP_LOGD(log_tag, "wrong checksum"); }
                    msg.clear();
                }
            }
        }
    }

    void state_diff(std::vector<byte> &p) {
        set_option_byte(status, heatstick_status_map, p, IDX_STATUS );
        set_option_byte(mode, heatstick_mode_map, p, IDX_MODE );
        set_option_byte(power, heatstick_power_map, p, IDX_POWER );
        set_option_byte(display, heatstick_display_map, p, IDX_DISPLAY );
        set_number_byte(temp_plan, p, IDX_TEMP_PLAN );
        set_sensor_byte(temp_real, p, IDX_TEMP_REAL );
        set_sensor_byte(power_lev, p, IDX_POWER_LEV );
        if(heatstick_state[0]==0xFF) {
            heatstick_state.swap(p);
            heatstick_state[IDX_CMD] = CMD_SET; // меняем команду в пакете
        }
    }

    void set_option_byte( template_::TemplateSelect *o, std::map<byte, std::string> &m, std::vector<byte> &p, byte i ) {
        if(p[i] != heatstick_state[i]) {
            auto call = id(o).make_call();
            call.set_option(m.at(p[i]));
            call.perform();
            heatstick_state[i] = p[i];
        }
    }

    void set_number_byte( template_::TemplateNumber *o, std::vector<byte> &p, byte i ) {
        if(p[i] != heatstick_state[i]) {
            auto call = id(o).make_call();
            call.set_value(p[i]);
            call.perform();
            heatstick_state[i] = p[i];
        }
    }

    void set_sensor_byte( template_::TemplateSensor *o, std::vector<byte> &p, byte i ) {
        if(p[i] != heatstick_state[i]) {
            id(o).publish_state(p[i]);
            heatstick_state[i] = p[i];
        }
    }

};
