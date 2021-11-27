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
parser.add_argument("-p","--ph-test", help="pH test", action='store_true', required=False)
parser.add_argument("-o","--orp-test", help="ORP test", action='store_true', required=False)
parser.add_argument("-e","--ec-test", help="EC test", action='store_true', required=False)
parser.add_argument("-t","--temp-test", help="temp test", action='store_true', required=False)
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
ph = { "val": int(), "str_val": str(), "calib_data": numpy.float32, "error_count": 0, 
    "name": PH_STR, "eeprom_calib_addr": 0x04}
adc_ph = MCP342x(i2c, 0x68, resolution=16)

ORP_STR = bcolors.BLUE + "ORP" + bcolors.ENDC
orp = { "val": None, "str_val": None, "calib_data": numpy.float32, "error_count": 0,
    "name": ORP_STR, "eeprom_calib_addr": 0x08}
adc_orp = MCP342x(i2c, 0x69, resolution=16)

EC_STR = bcolors.GREEN + "EC" + bcolors.ENDC
ec = { "val": None, "str_val": None, "calib_data": numpy.float32, "error_count": 0,
    "name": EC_STR, "eeprom_calib_addr": 0x00 }
adc_ec = MCP342x(i2c, 0x6A, resolution=16, gain=2)

RTD_STR = bcolors.BOLD + "RTD" + bcolors.ENDC
rtd = { "val": None, "str_val": None, "calib_data": numpy.float32, "error_count": 0, \
    "name": RTD_STR, "eeprom_calib_addr": 0x0c}
adc_rtd = MCP342x(i2c, 0x6B, resolution=16, gain=1)


def validate_val(val, target_val, error_margin):
    min = target_val - error_margin
    max = target_val + error_margin

    if (val["val"] < min) or (val["val"] > max):
        val["error_count"] = val["error_count"] + 1
        val["str_val"] = bcolors.FAIL + str(val["val"]) + bcolors.ENDC
    else:
        val["str_val"] = bcolors.OKGREEN + str(val["val"]) + bcolors.ENDC
    
    error = val["val"] - target_val

    return numpy.float32(error)


def read_and_validate(val, test_val, error_margin, adc):
    print('values (Volt) for ' + str(TEST_RUN_COUNT) + ' runs: ')
    for _ in range(0, TEST_RUN_COUNT):
        adc_reading = MCP342x.convert_and_read(adc, aggregate=numpy.mean, samples=ADC_SAMPLES)

        val["val"] = adc_reading
        validate_val(val, test_val, error_margin)
        print(val["name"] + ": " + val["str_val"])


def write_calib_data_to_eeprom(val):
    # convert float to little endian bytearray
    val_bytearray = bytearray(struct.pack('<f', val["calib_data"]))
    eeprom.extension_data_write(val_bytearray, val["eeprom_calib_addr"])


def read_calib_data_from_eeprom(val):
    val_bytearray = eeprom.extension_data_read(struct.calcsize('<f'), val["eeprom_calib_addr"])
    calib_data = struct.unpack('<f', val_bytearray)
    return numpy.float32(calib_data[0])


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
    ec_calib_data_readings = []
    for _ in range(0, TEST_RUN_COUNT):
        ec_calib_data_readings = ec_calib_data_readings + \
            MCP342x.convert_and_read(adc_ec, samples=ADC_SAMPLES)

    ec["calib_data"] = numpy.mean(ec_calib_data_readings)
    print("Read " + str(ADC_SAMPLES) + " from " + str(TEST_RUN_COUNT) + " test runs. "+ \
        "Average " + EC_STR + " disconnected value (float32): " + bcolors.OKGREEN + str(ec["calib_data"]) + bcolors.ENDC)

    # Test EC with 1K Resistor probe
    print("\n====================== " + EC_STR + " Testing =======================")
    print(bcolors.WARNING + "Connect 1K Resistor test probe to " + EC_STR + bcolors.WARNING + " terminal!" + bcolors.ENDC)
    input("Press Enter when ready to proceed with the test...")

    read_and_validate(ec, ec["calib_data"]*2, 0.008, adc_ec)
    
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
    for _ in range(0, TEST_RUN_COUNT):
        # Create a list of all the objects. They will be sampled in this
        # order, unless any later objects can be sampled can be moved earlier
        # for simultaneous sampling.
        adcs = [adc_ph, adc_orp, adc_rtd]
        r = MCP342x.convert_and_read_many(adcs, samples=ADC_SAMPLES, aggregate=numpy.mean)
        
        ph["val"] = r[0]
        orp["val"] = r[1]
        rtd["val"] = r[2]
        ph["calib_data"] = validate_val(ph, 1.25, 0.015)
        orp["calib_data"] = validate_val(orp, 1.25, 0.008)
        rtd["calib_data"] = validate_val(rtd, 1.25/2, 0.04)
        hydrokit_vals = ph["name"] + ": " + ph["str_val"] + \
            " (calib_data: " + str(ph["calib_data"]) + "V)" + " | " + \
            orp["name"] + ": " + orp["str_val"] + \
            " (calib_data: " + str(orp["calib_data"]) + "V)" + " | " + \
            rtd["name"] + ": " + rtd["str_val"] + \
            " (calib_data: " + str(rtd["calib_data"]) + "V)"
        print(hydrokit_vals)

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
        print("Writting calibration values to EEPROM:")
        calib_vals = ph["name"] + ": " + str(ph["calib_data"]) + " | "
        calib_vals += orp["name"] + ": " + str(orp["calib_data"]) + " | "
        calib_vals += ec["name"] + ": " + str(ec["calib_data"]) + " | "
        calib_vals += rtd["name"] + ": " + str(rtd["calib_data"])
        print(calib_vals)

        write_calib_data_to_eeprom(ph)
        write_calib_data_to_eeprom(orp)
        write_calib_data_to_eeprom(ec)
        write_calib_data_to_eeprom(rtd)

        # read values and to validate
        ph_offset_eeprom = read_calib_data_from_eeprom(ph)
        orp_offset_eeprom = read_calib_data_from_eeprom(orp)
        ec_offset_eeprom = read_calib_data_from_eeprom(ec)
        rtd_offset_eeprom = read_calib_data_from_eeprom(rtd)

        if (float(ph_offset_eeprom) == float(ph["calib_data"])) and \
            (float(orp_offset_eeprom) == float(orp["calib_data"])) and \
            (float(ec_offset_eeprom) == float(ec["calib_data"])) and \
            (float(rtd_offset_eeprom) == float(rtd["calib_data"])):
            print("HydroKit factory calibration: "+ bcolors.OKGREEN + "Success!\n" + bcolors.ENDC)
        else:
            print("HydroKit factory calibration: "+ bcolors.FAIL + "FAILED when writting to EEPROM!\n" + bcolors.ENDC)
            retval = False

    return retval


def main():
    if args.init:
        return hydrokit_init()

    if args.ph_test:
        print("\n===================== " + PH_STR + " Testing ========================")
        standard_slope = -59.18
        ph_calib_data = read_calib_data_from_eeprom(ph)
        for c in range(0, TEST_RUN_COUNT):
            ph["val"] = MCP342x.convert_and_read(adc_ph, aggregate=numpy.mean, samples=ADC_SAMPLES)
            ph_v = ph["val"] - 1.250 - ph_calib_data
            ph_val = 7 - (- ph_v*1000 / standard_slope)
            print(ph["name"] + "(RAW V): " + str(ph["val"]) + " | " + ph["name"] + "(V): " + str(ph_v) + " | " + ph["name"] + "(pH Val): " + str(ph_val))

    if args.orp_test:
        print("\n===================== " + ORP_STR + " Testing =======================")
        orp_calib_data = read_calib_data_from_eeprom(orp)
        for c in range(0, TEST_RUN_COUNT):
            orp["val"] = MCP342x.convert_and_read(adc_orp, aggregate=numpy.mean, samples=ADC_SAMPLES)
            orp_v = orp["val"] - 1.250 - orp_calib_data
            print(orp["name"] + "(RAW V): " + str(orp["val"]) + " | " + orp["name"] + "(V): " + str(orp_v))

    if args.ec_test:    
        print("\n===================== " + EC_STR + " Testing ========================")
        ec_calib_data = read_calib_data_from_eeprom(ec)
        print("Read 'ec_calib_data' val from EEPROM: " + str(ec_calib_data))
        k = 1.0
        for c in range(0, TEST_RUN_COUNT):
            ec["val"] = MCP342x.convert_and_read(adc_ec, aggregate=numpy.mean, samples=ADC_SAMPLES)
            ec_v = ec["val"]
            gain = ec_v / ec_calib_data
            resistance = 1000 / (gain - 1)
            uS = 1000000 * k / resistance
            print(ec["name"] + "(RAW V): " + str(ec["val"]) + " | " + ec["name"] + "(V): " + str(ec_v) + " | " + ec["name"] + "(uS): " + str(uS))

    if args.temp_test:
        print("\n===================== " + RTD_STR + " Testing =======================")
        rtd_calib_data = read_calib_data_from_eeprom(rtd)
        for c in range(0, TEST_RUN_COUNT):
            rtd["val"] = MCP342x.convert_and_read(adc_rtd, aggregate=numpy.mean, samples=ADC_SAMPLES)
            rtd_v = rtd["val"] - rtd_calib_data
            print(rtd["name"] + "(RAW V): " + str(rtd_v))

    print("=========================================================")


if __name__ == "__main__":
    ret = main()
    # Release the I2C bus
    i2c.unlock()

    exit(ret)
