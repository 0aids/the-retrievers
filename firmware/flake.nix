{
  description = "ESP-IDF development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # Community flake that packages the full ESP32 SDK and toolchain
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs =
    {
      self,
      nixpkgs,
      esp-dev,
    }:
    let
      # Define the architectures you want to support
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      # Helper function to generate an attrset for each system
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
    in
    {
      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          # Access the specific shell for the current system
          esp-shell = esp-dev.devShells.${system}.esp-idf-full;
          defaultPkgs = with pkgs; [
              pre-commit
              python313
              python313Packages.pytest
              python313Packages.pyserial
              python313Packages.asyncserial
          ];
        in
        {
          esp = esp-shell.overrideAttrs (oldShell: {
            nativeBuildInputs = (oldShell.nativeBuildInputs or [ ]) ++ [
              # Add extra packages here if needed:
              # pkgs.cmake
              # pkgs.ninja
            ] ++ defaultPkgs;
            shellHook = (oldShell.shellHook or "") + ''
            if [ "$(uname)" != "Darwin" ]; then
              export IDF_TOOLCHAIN=clang
            fi
            '';
          });

          # Ai thinker
          thinker =
                  pkgs.mkShell.override
        {
          # Override stdenv in order to change compiler:
          # stdenv = pkgs.clangStdenv;
        }
        {
          packages = with pkgs;
            [
                bear
                cmake
                clang-tools
                gcc-arm-embedded
                usbutils
                screen
                cmake-language-server
            ]++ defaultPkgs
            ++ (
              if system == "aarch64-darwin"
              then []
              else [gdb]
            );
          shellHook = ''
            export ARM_TOOLCHAIN_ROOT=${pkgs.gcc-arm-embedded}/arm-none-eabi/include
          '';
        };
          linux =
            pkgs.mkShell.override
              {
                # Override stdenv in order to change compiler:
                # stdenv = pkgs.clangStdenv;
              }
              {
                packages =
                  with pkgs;
                  [
                    bear
                    cmake
                    clang-tools
                    gcc-arm-embedded
                    usbutils
                    screen
                    cmake-language-server
                  ] ++ defaultPkgs
                  ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
                shellHook = ''
                  export ARM_TOOLCHAIN_ROOT=${pkgs.gcc-arm-embedded}/arm-none-eabi/include
                '';
              };
          default =
            pkgs.mkShell.override
              {
                # Override stdenv in order to change compiler:
                # stdenv = pkgs.clangStdenv;
              }
              {
                packages =
                  with pkgs;
                  [
                      clang-tools
                      pre-commit
                  ] ++ defaultPkgs
                  ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
                shellHook = ''
                  export ARM_TOOLCHAIN_ROOT=${pkgs.gcc-arm-embedded}/arm-none-eabi/include
                '';
              };
        }
      );
    };
}
