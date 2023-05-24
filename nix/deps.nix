{ nixpkgs ? <nixpkgs>}:

with import nixpkgs {};

rec {
  cereal = stdenv.mkDerivation rec {
    name = "cereal-${version}";
    version = "git-arximboldi-${commit}";
    commit = "2fe15c57f813db1b14c9b5e3e2389f7c5d1c5aff";
    src = fetchFromGitHub {
      owner = "flyqaq";
      repo = "cereal";
      rev = commit;
      sha256 = "119sldlzkrpnbb0kg052b851kifc7hwnc5vik1fdklramx5gzy97";
    };
    nativeBuildInputs = [ cmake ];
    cmakeFlags="-DJUST_INSTALL_CEREAL=true";
    meta = with lib; {
      homepage = "http://uscilab.github.io/cereal";
      description = "A C++11 library for serialization";
      license = licenses.bsd3;
    };
  };

  immer = stdenv.mkDerivation rec {
    name = "immer-${version}";
    version = "git-${commit}";
    commit = "a1271fa712342f5c6dfad876820da17e10c28214";
    src = fetchFromGitHub {
      owner = "arximboldi";
      repo = "immer";
      rev = commit;
      sha256 = "1bqkinkbp1b1aprg7ydfrbfs7gi779nypwvh9fj129frq1c2rxw5";
    };
    dontUseCmakeBuildDir = true;
    nativeBuildInputs = [ cmake ];
    meta = with lib; {
      homepage = "http://sinusoid.es/immer";
      description = "Immutable data structures for C++";
      license = licenses.lgpl3;
    };
  };

  zug = stdenv.mkDerivation rec {
    name = "zug-${version}";
    version = "git-${commit}";
    commit = "be20cae36e7e5876bf5bfb08b2a0562e1db3b546";
    src = fetchFromGitHub {
      owner = "arximboldi";
      repo = "zug";
      rev = commit;
      sha256 = "0vmcnspg9ys4qkj228kgvmpb5whly1cwx30sbg21x2iqs7y11ggx";
    };
    nativeBuildInputs = [ cmake ];
    dontUseCmakeBuildDir = true;
    meta = with lib; {
      homepage = "http://sinusoid.es/zug";
      description = "Transducers for C++";
    };
  };

  imgui = stdenv.mkDerivation rec {
    name = "imgui-${version}";
    version = "git-${commit}";
    commit = "1ebb91382757777382b3629ced2a573996e46453";
    src = fetchFromGitHub {
      owner = "ocornut";
      repo = "imgui";
      rev = commit;
      sha256 = "0zz4pb61dvrdlb7cmlfqid7m8jc583cipspg9dyj39w16h4z9bhx";
    };
    buildPhase = "";
    installPhase = ''
      mkdir $out
      cp $src/*.h $out/
      cp $src/*.cpp $out/
      cp $src/backends/imgui_impl_* $out/
    '';
    meta = with lib; {
      description = "Immediate mode UI library";
      license = licenses.lgpl3;
    };
  };

  elm = stdenv.mkDerivation rec {
    name = "elm-${version}";
    version = "0.19.1";
    src = fetchurl {
      url = "https://github.com/elm/compiler/releases/download/0.19.1/binary-for-linux-64-bit.gz";
      sha256 = "0p0m1xn4s4rk73q19fz1bw8qwhm6j3cqkrbq6jbmlwkzn8mzajp4";
    };
    nativeBuildInputs = [
      autoPatchelfHook
    ];
    unpackPhase = ''
      gunzip -c ${src} > elm
    '';
    installPhase = ''
      install -m755 -D elm $out/bin/elm
    '';
    meta = with lib; {
      homepage = "https://elm-lang.org";
      description = "A delightful language for reliable web applications";
    };
  };
}
