#!/usr/bin/env python3
from extension_utils import HyperExtensionEEPROM
import busio
import board
import argparse
import os
# use MCP2221 for adafruit-blinka
os.environ["BLINKA_MCP2221"] = "1"


parser = argparse.ArgumentParser()
parser.add_argument("-c", "--classid", type=str,
                    help="Hyper Class ID number to write to EEPROM", required=False)
args = parser.parse_args()

i2c = busio.I2C(board.SCL, board.SDA)
# Wait for I2C lock
while not i2c.try_lock():
    pass

eeprom = HyperExtensionEEPROM(i2c)


def main():

    if args.classid:
        hyper_class_id = int(args.classid)
        if (hyper_class_id < 1) or (hyper_class_id > 4294967295):
            print("Invalid Hyper Class ID passed as argument: " +
                  str(hyper_class_id))

    # Scan for devices on the I2C bus
    print("Scanning I2C bus")
    i2c_scan_list = []
    count = 0
    for x in i2c.scan():
        i2c_scan_list.append(hex(x))
        count += 1

    print("==================== I2C SCAN ====================")
    print("%d device(s) found on I2C bus:" % count)
    print(i2c_scan_list)

    # check if 24AA02E48 available in the bus
    if '0x50' and '0x51' not in i2c_scan_list:
        print("24AA02E48/24AA025E48/24AA02E64/24AA025E64 Device not found!")
        exit()

    # Read EUI-48 from EEPROM
    print("============= Extension EEPROM Info ==============")
    print("EUI-48: ", end='')
    eui48 = eeprom.eeprom_get_eui48()
    print("'" + eui48.hex(":") + "'")

    # Read Hyper Class ID from EEPROM
    if args.classid:
        if eeprom.hyper_class_id_read() == hyper_class_id:
            print("Hyper Class ID '" + str(hyper_class_id) +
                  "' already flashed to EEPROM, skipping... ")

        else:
            # Writting Hyper Class ID to EEPROM
            print("Writting Hyper Class ID '" +
                  str(hyper_class_id) + "' to EEPROM... ", end='')
            eeprom.hyper_class_id_write(hyper_class_id)
            print("Done!")

            # read Hyper Class ID from EEPROM to confirm it was correctly written
            if hyper_class_id != eeprom.hyper_class_id_read():
                print("\nHyper Class ID Write and Read mismatch!!! Exiting...")
                # Release the I2C bus
                i2c.unlock()
                exit()

    else:
        # Read Hyper Class ID from EEPROM
        print("Hyper Class ID: ", end='')
        hcidr = eeprom.hyper_class_id_read()
        print("'" + str(hcidr) + "'")

    print("==================================================")
    # Release the I2C bus
    i2c.unlock()


if __name__ == "__main__":
    main()
