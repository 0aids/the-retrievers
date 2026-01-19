{
  description = "ESP-IDF development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # Community flake that packages the full ESP32 SDK and toolchain
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs = { self, nixpkgs, esp-dev }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      # Choose your target: esp32-idf, esp32s3-idf, esp8266-rtos-sdk, etc.
      esp-shell = esp-dev.devShells.${system}.esp-idf-full;
    in
    {
      devShells.${system}.default = esp-shell.overrideAttrs (
          oldShell: {
              nativeBuildInputs = (oldShell.nativeBuildInputs or []) ++ [
                  # pkgs.clang-tools
                  # pkgs.gnumake
                  # pkgs.cmake
                  # pkgs.bear
                  #
                  pkgs.pre-commit
		  pkgs.cspell
		  pkgs.cppcheck
              ];
              shellHook = oldShell.shellHook + ''
                  export IDF_TOOLCHAIN=clang
              '';
          }
      );
    }
    ;
}

