{ pkgs ? import <nixpkgs> {}, extra ? [], root, gitignore ? "${root}/.gitignore" }:

let
  ignores = pkgs.lib.strings.fileContents gitignore
    + builtins.foldl' (acc: e: acc + "\n" + e) "\n" extra;
in pkgs.nix-gitignore.gitignoreSourcePure ignores root
