#!/usr/bin/env bash

pushd "$(dirname "$0")"

[[ -d build ]] && rm -r build
mkdir build || exit 1

. PKGBUILD.src

cp ../trixiekeys.c ../trixiekeys.h ../config.c ../trixiekeysd build/
cp PKGBUILD.src build/PKGBUILD

cd build

echo "md5sums=(" >> PKGBUILD
for FN in "${source[@]}"; do
    echo \'$(md5sum "$FN" | cut -f 1 -d ' ')\' >> PKGBUILD
done
echo ")" >> PKGBUILD

makepkg -f
makepkg -f --source

popd
