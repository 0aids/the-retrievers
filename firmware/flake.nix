{
  description = "ESP-IDF development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # Community flake that packages the full ESP32 SDK and toolchain
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs = { self, nixpkgs, esp-dev }:
    let
      # Define the architectures you want to support
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];

      # Helper function to generate an attrset for each system
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          # Access the specific shell for the current system
          esp-shell = esp-dev.devShells.${system}.esp-idf-full;
        in
        {
          default = esp-shell.overrideAttrs (oldShell: {
            nativeBuildInputs = (oldShell.nativeBuildInputs or [ ]) ++ [
              # Add extra packages here if needed:
              # pkgs.cmake
              # pkgs.ninja
            ];
            # shellHook = (oldShell.shellHook or "") + ''
            #   export IDF_TOOLCHAIN=clang
            # '';
          });
        }
      );
    };
}
