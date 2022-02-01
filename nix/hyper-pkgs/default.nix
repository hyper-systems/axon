let
  nixpkgs = import (builtins.fetchTarball {
    name = "hyper-nixpkgs";
    url = "https://github.com/NixOS/nixpkgs/archive/refs/tags/21.11.tar.gz";
    sha256 = "162dywda2dvfj1248afxc45kcrg83appjd0nmdb541hl7rnncf02";
  });

in nixpkgs {
  overlays = map (name: import (./overlays + "/${name}"))
    (builtins.attrNames (builtins.readDir ./overlays));
  
  config.allowUnfree = true;
}
