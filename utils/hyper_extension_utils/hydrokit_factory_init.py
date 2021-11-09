#!/usr/bin/env python3
import argparse
import os
# use MCP2221 for adafruit-blinka
os.environ["BLINKA_MCP2221"] = "1"
import time
import board
import busio
import numpy
import struct
from extension_utils import HyperExtensionEEPROM
from MCP342x import MCP342x

HYDROKIT_CLASS_ID = [9, 13]
ADC_SAMPLES=120

parser = argparse.ArgumentParser()
group = parser.add_mutually_exclusive_group()
group.add_argument("-i","--init", help="init Hydrokit", action='store_true', required=False)
group.add_argument("-p","--ph-test", help="pH test", action='store_true', required=False)
group.add_argument("-o","--orp-test", help="ORP test", action='store_true', required=False)
group.add_argument("-e","--ec-test", help="EC test", action='store_true', required=False)
group.add_argument("-t","--temp-test", help="temp test", action='store_true', required=False)
parser.add_argument("-c","--count", help="test loop count", default=3, required=False)
args = parser.parse_args()

TEST_RUN_COUNT = int(args.count)

class bcolors:
    HEADER = '\033[95m'
    BLUE = '\033[34m'
    LIGHT_BLUE = '\033[94m'
    OKBLUE = LIGHT_BLUE
    OKCYAN = '\033[96m'
    GREEN = '\033[32m'
    LIGHT_GREEN = '\033[92m'
    OKGREEN = LIGHT_GREEN
    YELLOW = '\033[93m'
    WARNING = YELLOW
    RED = '\033[31m'
    LIGHT_RED = '\033[91m'
    FAIL = LIGHT_RED
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


i2c = busio.I2C(board.SCL, board.SDA)
# Wait for I2C lock
while not i2c.try_lock():
    pass

eeprom = HyperExtensionEEPROM(i2c)

PH_STR = bcolors.RED + "pH" + bcolors.ENDC
ph = { "val": int(), "str": str(), "error_count": 0, "name": PH_STR}
adc_ph = MCP342x(i2c, 0x68, resolution=16)

ORP_STR = bcolors.BLUE + "ORP" + bcolors.ENDC
orp = { "val": None, "str": None, "error_count": 0, "name": ORP_STR}
adc_orp = MCP342x(i2c, 0x69, resolution=16)

EC_STR = bcolors.GREEN + "EC" + bcolors.ENDC
ec = { "val": None, "disconnected_val": int, "str": None, "error_count": 0, "name": EC_STR}
adc_ec = MCP342x(i2c, 0x6A, resolution=16, gain=2)

RTD_STR = bcolors.BOLD + "RTD" + bcolors.ENDC
rtd = { "val": None, "str": None, "error_count": 0, "name": RTD_STR}
adc_rtd = MCP342x(i2c, 0x6B, resolution=16, gain=2)


def validate_val(val, test_val, error_margin):
    min = test_val - error_margin
    max = test_val + error_margin

    if (val["val"] < min) or (val["val"] > max):
        val["error_count"] = val["error_count"] + 1
        val["str"] = bcolors.FAIL + str(val["val"]) + bcolors.ENDC
    else:
        val["str"] = bcolors.OKGREEN + str(val["val"]) + bcolors.ENDC


def read_and_validate(val, test_val, error_margin, adc):
    print('values (Volt) for ' + str(TEST_RUN_COUNT) + ' runs: ')
    for c in range(0, TEST_RUN_COUNT):
        adc_reading = MCP342x.convert_and_read(adc, aggregate=numpy.mean, samples=ADC_SAMPLES)
        time.sleep(0.5)

        val["val"] = adc_reading
        validate_val(val, test_val, error_margin)
        print(val["name"] + ": " + val["str"])


def write_ec_disconnected_to_eeprom(ec_disconnected):
    # convert float to little endian bytearray
    ec_bytearray = bytearray(struct.pack('<f', ec_disconnected))
    eeprom.extension_data_write(ec_bytearray)


def read_ec_disconnected_from_eeprom():
    ec_disconnected_eeprom = struct.unpack('<f', eeprom.extension_data_read(struct.calcsize('<f')))
    return ec_disconnected_eeprom[0]


def hydrokit_init():
 # check if extension is HydroKit available in the bus
    classid = eeprom.hyper_class_id_read()
    if classid not in HYDROKIT_CLASS_ID :
        print("HydroKit not detected! Hyper Class ID is : "+str(classid))
        exit(1)


    # EC Calibration
    print("=============== " + EC_STR + " Factory Calibration ==================")
    print(bcolors.WARNING + "Disconnect any probe from the " + EC_STR + bcolors.WARNING + " terminal!" + bcolors.ENDC)
    input("Press Enter when ready to proceed to obtain the disconnected value...")
    
    print("Calculating disconnected " + EC_STR + " value...")
    ec_disconnected_readings = []
    for c in range(0, TEST_RUN_COUNT):
        ec_disconnected_readings = ec_disconnected_readings + \
            MCP342x.convert_and_read(adc_ec, samples=ADC_SAMPLES)
        time.sleep(0.5)

    ec["disconnected_val"] = numpy.mean(ec_disconnected_readings)
    # convert to float32
    ec["disconnected_val"] = ec["disconnected_val"]
    print("Read " + str(ADC_SAMPLES) + " from " + str(TEST_RUN_COUNT) + " test runs. "+ \
        "Average " + EC_STR + " disconnected value (float32): " + bcolors.OKGREEN + str(ec["disconnected_val"]) + bcolors.ENDC)

    # write disconnected_val to eeprom
    write_ec_disconnected_to_eeprom(ec["disconnected_val"])

    # Test EC with 1K Resistor probe
    print("\n====================== " + EC_STR + " Testing =======================")
    print(bcolors.WARNING + "Connect 1K Resistor test probe to " + EC_STR + bcolors.WARNING + " terminal!" + bcolors.ENDC)
    input("Press Enter key when ready to proceed with the test...")

    # read disconnected_val from eeprom
    ec["disconnected_val"] = read_ec_disconnected_from_eeprom()
    print("Read 'ec_disconnected' val from EEPROM: " + str(ec["disconnected_val"]))
    read_and_validate(ec, ec["disconnected_val"]*2, 0.008, adc_ec)
    
    if ec["error_count"] > 0:
        print("Wrong test " + EC_STR + " value! Faulty hardware, test probe not attached or wrong test probe!")
        return 1
    else:
        print("EC 1K Resistor test probe test: "+ bcolors.OKGREEN + "Success!\n" + bcolors.ENDC)

    # Test EC with Shunt probe
    print(bcolors.WARNING + "Connect Shunt test probe to " + EC_STR + bcolors.WARNING + " terminal!" + bcolors.ENDC)
    input("Press Enter when ready to proceed with the test...")

    read_and_validate(ec, 1.024, 0.004, adc_ec)

    if ec["error_count"] > 0:
        print("Wrong test " + EC_STR + " value! Faulty hardware, test probe not attached or wrong test probe!")
        return 1
    else:
        print("EC Shunt test probe test: "+ bcolors.OKGREEN + "Success!\n" + bcolors.ENDC)

    # Test pH amd ORP with Shunt test probe, and RTD with 1K Resistor probe.
    print("\n=============== " + PH_STR + ", " + ORP_STR + " and " + RTD_STR + " Testing =================")
    print(bcolors.WARNING + "Before proceeding, make sure the following Test probes are connected to the terminals: " + bcolors.ENDC)
    print("- " + PH_STR + " and " + ORP_STR + " with Shunt Test probe.")
    print("- " + RTD_STR + " with 1K Resistor Test probe.")
    input("\nPress Enter when ready to proceed with the test...")

    print('values (Volt) for ' + str(TEST_RUN_COUNT) + ' runs: ')
    for c in range(0, TEST_RUN_COUNT):
        # Create a list of all the objects. They will be sampled in this
        # order, unless any later objects can be sampled can be moved earlier
        # for simultaneous sampling.
        adcs = [adc_ph, adc_orp, adc_rtd]
        r = MCP342x.convert_and_read_many(adcs, samples=ADC_SAMPLES, aggregate=numpy.mean)
        
        ph["val"] = r[0]
        orp["val"] = r[1]
        rtd["val"] = r[2]
        validate_val(ph, 1.25, 0.003)
        validate_val(orp, 1.25, 0.003)
        validate_val(rtd, 1.25/2, 0.01)
        hydrokit_vals = ph["name"] + ": " + ph["str"] + ", " + \
            orp["name"] + ": " + orp["str"] + ", " + rtd["name"] + ": " + rtd["str"]
        
        print(hydrokit_vals)
        time.sleep(1)

    retval = True
    if ph["error_count"] > 0:
        print("Wrong test pH value, faulty hardware or test probe not attached!")
        retval = False
    if orp["error_count"] > 0:
        print("Wrong test ORP value, faulty hardware or test probe not attached!")
        retval = False
    if ec["error_count"] > 0:
        print("Wrong test " + EC_STR + " value, faulty hardware or test probe not attached!")
        retval = False
    if rtd["error_count"] > 0:
        print("Wrong test RTD value, faulty hardware or test probe not attached!")
        retval = False

    print("=========================================================")

    if retval:
        print("HydroKit test: "+ bcolors.OKGREEN + "Success!\n" + bcolors.ENDC)

    return retval


def main():
    if args.init:
        return hydrokit_init()

    if args.ph_test:
        print("\n===================== " + PH_STR + " Testing ========================")
        standard_slope = -59.18
        for c in range(0, TEST_RUN_COUNT):
            ph["val"] = MCP342x.convert_and_read(adc_ph, aggregate=numpy.mean, samples=ADC_SAMPLES)
            ph_mv = ph["val"] - 1.250
            ph_val = 7 - (- ph_mv*1000 / standard_slope)
            print(ph["name"] + "(V): " + str(ph_mv) + " | " + ph["name"] + "(pH Val): " + str(ph_val))

        print("=========================================================")

    elif args.orp_test:
        print("\n===================== " + ORP_STR + " Testing =======================")
        for c in range(0, TEST_RUN_COUNT):
            orp["val"] = MCP342x.convert_and_read(adc_orp, aggregate=numpy.mean, samples=ADC_SAMPLES)
            orp_mv = orp["val"] - 1.250
            print(orp["name"] + "(V): " + str(orp_mv))

        print("=========================================================")

    elif args.ec_test:    
        print("\n===================== " + EC_STR + " Testing ========================")
        ec_disconnected = read_ec_disconnected_from_eeprom()
        print("Read 'ec_disconnected' val from EEPROM: " + str(ec_disconnected))
        k = 1.0
        for c in range(0, TEST_RUN_COUNT):
            ec["val"] = MCP342x.convert_and_read(adc_ec, aggregate=numpy.mean, samples=ADC_SAMPLES)
            ec_mv = ec["val"]
            gain = ec_mv / ec_disconnected
            resistance = 1000 / (gain - 1)
            uS = 1000000 * k / resistance
            print(ec["name"] + "(V): " + str(ec_mv) + " | " + ec["name"] + "(uS): " + str(uS))

        print("=========================================================")
 

    elif args.temp_test:
        print("\n===================== " + RTD_STR + " Testing =======================")
        print("=========================================================")


if __name__ == "__main__":
    ret = main()
    # Release the I2C bus
    i2c.unlock()

    exit(ret)
