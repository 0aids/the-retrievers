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
    rec {
      devShells.${system}.default = esp-shell.overrideAttrs (
          oldShell: {
              nativeBuildInputs = (oldShell.nativeBuildInputs or []) ++ [
                  pkgs.gnumake
                  pkgs.cmake
                  pkgs.bear
                  pkgs.usbutils
                  pkgs.gcc-arm-embedded
                  pkgs.python313
                  pkgs.uv
              ];
          shellHook = (oldShell.shellHook or "") + ''
            # 2. Verify it worked (Optional, helps debugging)
            echo "Using clangd from: $(which clangd)"

            # 3. Set your env var
            export IDF_TOOLCHAIN=clang
            export CFLAGS="$CFLAGS \
                -I$IDF_PATH/components/newlib/platform_include \
                -I$IDF_PATH/components/newlib/include \
                -I$PWD/build/config"
          '';

          }
      );
    }
    ;
}
