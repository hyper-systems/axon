# F


## Usage

First get into a `nix-shell` by doing:

```shell
$ nix-shell
```

Inside the `nix-shell` run the desired scripts. Example:

```shell
[nix]$ ./extension_eeprom_flashing_tool.py --help
```


## Special setup steps in Linux

Do the following before connecting the device.

```shell
$ echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="04d8", ATTR{idProduct}=="00dd", MODE="0666"' | sudo tee /etc/udev/rules.d/99-mcp2221.rules
$ sudo rmmod hid_mcp2221
$ echo 'blacklist hid_mcp2221' | sudo tee -a /etc/modprobe.d/blacklist.conf
$ sudo update-initramfs -u
```