{
  description = "Flake for Esp32 Development";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.11";
    nixpkgs-unstable.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    esp32-nix.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs =
    { self
    , nixpkgs
    , nixpkgs-unstable
    , flake-utils
    , esp32-nix
    ,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs-unstable { inherit system; };
        espPkgs = import nixpkgs {
          inherit system;
          overlays = [ esp32-nix.overlays.default ];
          config = {
            permittedInsecurePackages = [
              "python3.13-ecdsa-0.19.1"
            ];
          };
        };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "esp32";

          nativeBuildInputs = with pkgs; [
            clang-tools
            cmake
            cmake-language-server
          ];

          buildInputs = [
            espPkgs.esp-idf-xtensa
          ];

          shellHook = ''
            echo -e "\033[0;32mDone!\033[0m"
          '';
        };
      }
    );
}
