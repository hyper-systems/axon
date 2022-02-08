{ pkgs ? import ./nix/hyper-pkgs }:

with pkgs;

mkShell {
  nativeBuildInputs = [ fzf ];
  buildInputs = [
      which
      cmake
      git
      ninja
      gperf
      ccache
      dfu-util
      dtc
      python3.pkgs.virtualenv
      python3.pkgs.setuptools
      gcc-arm-embedded-zephyr
    ] ++ lib.optionals stdenv.isLinux [ segger-jlink nrf-command-line-tools ];

  GNUARMEMB_TOOLCHAIN_PATH = "${gcc-arm-embedded-zephyr}";
  PROJECT_ROOT = builtins.toString ./.;
  shellHook = ''
    export PS1="[\[\033[1;34m\]nix\[\033[0m\]] $PS1"
    if command -v fzf-share >/dev/null; then
      source "$(fzf-share)/key-bindings.bash"
      source "$(fzf-share)/completion.bash"
    fi
    ${if stdenv.isDarwin then ''
        if ! command -v JLinkGDBServer > /dev/null; then
            echo "Install 'segger-jlink' for macOS manually from: https://www.segger.com/downloads/jlink."
            echo "Alternatively install 'segger-jlink' it with Homebrew: 'brew install segger-jlink'"
            exit 1
        fi
        if ! command -v nrfjprog > /dev/null; then
            echo "Install 'nrf-command-line-tools' for macOS manually from: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download"
            exit 1
        fi
        # pyocd fails to install due to capstone build failure.
        # somehow the nix clang wrapper is not enough
        # link /usr/bin/clang and inject it into the environment
        mkdir -p $PROJECT_ROOT/.nix/bin
        ln -sf /usr/bin/clang $PROJECT_ROOT/.nix/bin/clang
        export PATH="$PROJECT_ROOT/.nix/bin:$PATH"
    '' else ''
      if [ ! -f /etc/udev/rules.d/99-jlink.rules ]; then
        printf "Installing udev rule for segger-jlink at '/etc/udev/rules.d/99-jlink.rules'... "
        sudo cp "${segger-jlink}/lib/udev/rules.d/99-jlink.rules" /etc/udev/rules.d/99-jlink.rules && \
        echo "OK"
      fi
      # inject segger-jlink lib path into LD_LIBRARY_PATH so that nrfjprog is happy
      export LD_LIBRARY_PATH="${segger-jlink}/lib:$LD_LIBRARY_PATH"''}
  '';
}
