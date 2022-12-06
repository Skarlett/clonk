{
  description = "Clonk! An amature tree walking interpreter from first principles.";

  # Nixpkgs / NixOS version to use.
  inputs.nixpkgs.url = "nixpkgs/nixos-22.11";

  outputs = { self, nixpkgs }:
    let
      lastModifiedDate = self.lastModifiedDate or self.lastModified or "19700101";
      version = builtins.substring 0 8 lastModifiedDate;
      supportedSystems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; overlays = [ self.overlay ]; });
    in
    {
      overlay = final: prev:
        let
            dependencies =  with prev; [ cmake libunwind.dev libunwind gnumake clang ];
        in
        with final; {
          clonk =  stdenv.mkDerivation
            {
              name = "clonk-cc";
              src = ./.;
              nativeBuildInputs = dependencies;
            };

          clonk-test = stdenv.mkDerivation
            {
              name = "clonk-test";
              src = ./.;
              nativeBuildInputs = dependencies;
            };
        };

      packages = forAllSystems (system:
        {
          inherit (nixpkgsFor.${system}) clonk;
          inherit (nixpkgsFor.${system}) clonk-test;
        });

      devShells = forAllSystems (system: self.packages.${system}.clonk);
      defaultPackage = forAllSystems (system: self.packages.${system}.clonk);
    };
}
