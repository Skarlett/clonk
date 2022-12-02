# python.nix
# https://github.com/davhau/mach-nix
#
with (import <nixpkgs> {});
mkShell {
  buildInputs = [
      cmake
      ripgrep
      clang-tools
  ];

  # shellHook = ''
  #   export DEBUG=1
  # '';
}
