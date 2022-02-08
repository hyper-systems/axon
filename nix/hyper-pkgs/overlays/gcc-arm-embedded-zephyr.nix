self: super: {
  gcc-arm-embedded-zephyr = super.gcc-arm-embedded-9.overrideAttrs (attrs: rec {
    version = "9.3.1";
    release = "9-2019-q4-major";
    subdir = "9-2019q4";

    src = super.fetchurl {
      url =
        "https://developer.arm.com/-/media/Files/downloads/gnu-rm/${subdir}/gcc-arm-none-eabi-${release}-${attrs.suffix}.tar.bz2";
      sha256 = {
        aarch64-darwin = "1249f860d4155d9c3ba8f30c19e7a88c5047923cea17e0d08e633f12408f01f0";
        aarch64-linux = "1f5b9309006737950b2218250e6bb392e2d68d4f1a764fe66be96e2a78888d83";
        x86_64-darwin = "1249f860d4155d9c3ba8f30c19e7a88c5047923cea17e0d08e633f12408f01f0";
        x86_64-linux = "bcd840f839d5bf49279638e9f67890b2ef3a7c9c7a9b25271e83ec4ff41d177a";
      }.${super.stdenv.hostPlatform.system} or (throw
        "Unsupported system: ${super.stdenv.hostPlatform.system}");
    };
  });
}
