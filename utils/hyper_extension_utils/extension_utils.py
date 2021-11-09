
class HyperExtensionEEPROM(object):
    """
    Class to represent Hyper Extension EEPROMs.
    """
    _EEPROM_24AA02E48_I2C_ADDR = 0x50
    

    def __init__(self, i2c):
        self.i2c = i2c
        _i2c_scan_list = self.i2c.scan()
        if self._EEPROM_24AA02E48_I2C_ADDR not in self.i2c.scan():
            print("I2C scan list: " + str([hex(x) for x in _i2c_scan_list]) + \
                "\n'24AA02E48/24AA025E48/24AA02E64/24AA025E64' I2C EEPROM Device not found!")
            exit(1)
        else:
            self.i2c_eeprom_addr = self._EEPROM_24AA02E48_I2C_ADDR
    

    def eeprom_read(self, len, pointer):
        buffer = bytearray(len) 
        # set memory pointer to 'pointer'
        self.i2c.writeto(self.i2c_eeprom_addr, bytes([pointer]))
        #buffer = bytearray(buffer)
        self.i2c.readfrom_into(self.i2c_eeprom_addr+1, buffer)

        return buffer


    def eeprom_write(self, buffer, pointer):
        if type(buffer) is int:
            buffer = [buffer]
        self.i2c.writeto(self.i2c_eeprom_addr, bytearray([pointer])+bytearray(buffer))


    def eeprom_get_eui48(self):
        # Read EUI-48 from device
        eui48 = self.eeprom_read(6, 0xFA)
        #print([hex(x) for x in eui48])

        return eui48

    def hyper_class_id_write(self, hyper_class_id):
        # set pointer to 0x00 and write byte corresponding to hyper_class_id
        buffer = bytearray(hyper_class_id.to_bytes(4,byteorder='little'))
        self.eeprom_write(buffer, 0x00)


    def hyper_class_id_read(self):
        buffer = self.eeprom_read(4, 0x00)
        hcidr = int.from_bytes(buffer, byteorder='little', signed=False)
        #print(str([hex(x) for x in buffer]) + " (uint32_t: " + str(hcidr) +")")

        return hcidr

    def extension_data_write(self, data):
        # check if size is bigger than 230 bytes
        try:
            if len(data) > 230:
                raise ValueError("extension data to be written can't be bigger than 230 bytes!")
            else:
                # write data from 0x14 offset,
                # first 20 bytes are reserved for special Hyper data
                self.eeprom_write(bytearray(data), 0x14)
        except ValueError as e:
            print(e)
            exit(1)

    def extension_data_read(self, size):
        # check if size is bigger than 230 bytes
        try:
            if size > 230:
                raise ValueError("extension data to be read can't be bigger than 230 bytes!")
            else:
                # read data from 0x14 offset,
                # first 20 bytes are reserved for special Hyper data
                return self.eeprom_read(size, 0x14)
        except ValueError as e:
            print(e)
            exit(1)
