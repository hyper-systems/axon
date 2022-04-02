{ pkgs ? import ../../nix/hyper-pkgs }:

with pkgs;

let
  setupVenv = ''
    if [ ! -f .venv/.installed ]; then
      rm -rf .venv \
      && python3 -m venv .venv \
      && source .venv/bin/activate \
      && pip3 install --upgrade pip \
      && pip3 install --verbose -r requirements.txt \
      && touch .venv/.installed \
      || exit 1
    fi
    source .venv/bin/activate
      '';
  mcp2221ShellHook = ''
    # use MCP2221 for adafruit-blinka
    export BLINKA_MCP2221=1
    ${if stdenv.isLinux then ''
      if [ ! -f /etc/udev/rules.d/99-mcp2221.rules ]; then
        printf "Installing udev rule for mcp2221 at '/etc/udev/rules.d/99-mcp2221.rules':"
        echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="04d8", ATTR{idProduct}=="00dd", MODE="0666"' | \
          sudo tee /etc/udev/rules.d/99-mcp2221.rules && echo "Done!"
      fi
      if ! grep -q 'blacklist hid_mcp2221' /etc/modprobe.d/blacklist.conf; then
        echo "Blacklisting hid_mcp2221 Kernel module:"
        sudo rmmod hid_mcp2221 > /dev/null 2>&1
        echo 'blacklist hid_mcp2221' | sudo tee -a /etc/modprobe.d/blacklist.conf
        sudo update-initramfs -u
        echo "Done!"
      fi
    '' else
      ""}
      '';

in mkShell {
  nativeBuildInputs = with buildPackages; [ which ];
  buildInputs = [
    python3
    python3.pkgs.virtualenv
    python3.pkgs.setuptools
    python3.pkgs.wheel
    # the following need a patch in nix, use the native packages
    python3.pkgs.numpy
    python3.pkgs.hidapi
  ];
  shellHook = ''
    export PS1="[\[\033[1;34m\]nix\[\033[0m\]] $PS1"
    ${mcp2221ShellHook}
    ${setupVenv}
  '';
}
