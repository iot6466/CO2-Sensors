#! /usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import division # to calculate a float from integers
import smbus, time

import os, sys
from datetime import datetime 
import paho.mqtt.client as mqtt

def crc_calc(data):
    return 0xFF - (data[0] + data[1] + data[2] + data[3] + data[4] + data[5]) % 0xFF

def main():
    
    dt = datetime.now().isoformat(' ')

    # must instantiate the bus. 
    # on RPi 256 MB version, it's called i2c_0
    # on RPi 512 MB version, it's called i2c_1
    i2c = smbus.SMBus(1)
    time.sleep(1.0)
    
    # sensors with their bus addresses, see above
    sensor = 0x70
    crc   = 1
    retry = 19
    voc   = 0
    co2   = 400
    data_r = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    while (retry > 0) and (crc != data_r[6]):
        retry = retry - 1

        dt = datetime.now().isoformat(' ')
        data_w = [0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        data_w[6] = crc_calc(data_w)
        res = i2c.write_i2c_block_data(sensor, data_w[0], data_w[1:6])
        time.sleep(0.01)
        data_r[0] = i2c.read_byte(sensor)
        data_r[1] = i2c.read_byte(sensor)
        data_r[2] = i2c.read_byte(sensor)
        data_r[3] = i2c.read_byte(sensor)
        data_r[4] = i2c.read_byte(sensor)
        data_r[5] = i2c.read_byte(sensor)
        data_r[6] = i2c.read_byte(sensor)
       
        voc = (data_r[0] - 13) * (1000 / 229) 
        co2 = (data_r[1] - 13) * (1600 / 229) + 400
        crc = crc_calc(data_r)
        print co2, voc, data_r, crc
        if (crc != data_r[6]):
            i2c = smbus.SMBus(1)
            time.sleep(1.0)

    if (crc == data_r[6]):
        try:
            client = mqtt.Client()
            client.connect("rasp-ssd-1", 1883, 60)
            client.loop(100)
            client.publish(topic="/rbpt-2/datetime", payload="%.16s"%(dt), qos=0, retain=True)
            client.loop(100)
            client.publish(topic="/rbpt-2/VZ89/VOC", payload="%.2f"%(voc), qos=0, retain=True)
            client.loop(100)
            client.publish(topic="/rbpt-2/VZ89/CO2", payload="%.2f"%(co2), qos=0, retain=True)
            client.loop(100)
        except:
            print "error mqtt"
    else:
        print "bad reading"

if __name__ == "__main__":
    main()
