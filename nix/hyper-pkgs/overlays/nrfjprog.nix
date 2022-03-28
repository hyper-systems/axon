# drop this when https://github.com/NixOS/nixpkgs/pull/121601 is merged

self: super: {

  nrf-command-line-tools = (let
    supported = {
      x86_64-linux = {
        name = "amd64";
        sha256 =
          "1567a09bdf76b1f6290d27d53f4aba3e5ed4ac52a461e6a0c1b15f8e2198560c";
      };
    };

    platform = supported.${super.system} or (throw
      "unsupported platform ${super.system}");

    version = "10.15.4";
    subdir = "Versions-10-x-x/10-15-4";

    url =
      "https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/${subdir}/nrf-command-line-tools-${version}_Linux-${platform.name}.tar.gz";

  in super.stdenv.mkDerivation {
    pname = "nrf-command-line-tools";
    inherit version;
    src = super.fetchurl {
      inherit url;
      inherit (platform) sha256;
    };
    runtimeDependencies = [ super.segger-jlink ];
    dontConfigure = true;
    dontBuild = true;
    installPhase = ''
      mkdir -p $out
      cp -r * $out/
    '';
    meta = with super.lib; {
      description = "nRF Command Line Tools";
      homepage =
        "https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrf_cltools%2FUG%2Fcltools%2Fnrf_command_line_tools_lpage.html";
      license = licenses.unfree;
      platforms = attrNames supported;
    };
  });
}
