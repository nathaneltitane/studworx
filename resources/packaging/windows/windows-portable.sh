#!/bin/bash

# build a studworx windows portable self extractor from a release build, run in git bash after nmake
# usage: resources/packaging/windows/windows-portable.sh [build directory]

set -e

directory_script="${BASH_SOURCE[0]%/*}"

# run from its own directory
if [ "${directory_script}" = "${BASH_SOURCE[0]}" ]
then
	directory_script="."
fi

directory_source="$(cd "${directory_script}/../../.." && pwd)"
directory_build="${1:-${directory_source}/build/windows}"

# version from the release date
while read -r line_header
do
	if [[ ${line_header} =~ SW_VERSION_TEXT\ \"([0-9-]+)\" ]]
	then
		package_version="${BASH_REMATCH[1]}"
	fi
done < "${directory_source}/source/application/sw_global.h"

if [ -z "${package_version}" ]
then
	printf "%s\n" "version not found"
	exit 1
fi

# stage the portable files
mkdir -p "${directory_build}"
directory_build="$(cd "${directory_build}" && pwd)"
directory_package="$(mktemp -d "${directory_build}/portable.XXXXXX")"

cp "${directory_source}/build/release/StudWorx.exe" "${directory_package}/"
cp "${directory_source}/README.md" "${directory_package}/readme.txt"
mkdir -p "${directory_package}/samples"
cp "${directory_source}/resources/samples/"*.ldr "${directory_source}/resources/samples/"*.mpd "${directory_package}/samples/"

for file_extra in povconsole32-sse2.exe
do
	if [ -f "${directory_source}/${file_extra}" ]
	then
		cp "${directory_source}/${file_extra}" "${directory_package}/"
	fi
done

windeployqt --release "${directory_package}/StudWorx.exe"

# runtime libraries beside the program, vc_redist needs admin rights
directory_runtime=("$(cygpath -u "${VCToolsRedistDir}")x64/Microsoft.VC"*".CRT")

if [ ! -d "${directory_runtime[0]}" ]
then
	printf "%s\n" "visual c++ runtime not found"
	exit 1
fi

cp "${directory_runtime[0]}/"*.dll "${directory_package}/"

# settings folder switches the program to ini settings
mkdir -p "${directory_package}/settings"
printf "%s\n" "StudWorx keeps its settings in this folder while it exists." > "${directory_package}/settings/readme.txt"

# write the self extractor script
file_script="${directory_build}/studworx-portable.nsi"
file_package="${directory_build}/studworx-windows-${package_version}-x64-portable.exe"

cat > "${file_script}" << EOF
!include "MUI2.nsh"

Name "StudWorx Portable"
OutFile "${file_package##*/}"
Unicode True
RequestExecutionLevel user
InstallDir "\$DESKTOP\\StudWorx"
SetCompressor /SOLID lzma

!define MUI_ABORTWARNING
!define MUI_ICON "setup.ico"
!define MUI_DIRECTORYPAGE_TEXT_TOP "Choose a folder to extract StudWorx into, such as one on a USB drive. StudWorx keeps its settings in the settings folder inside it."
!define MUI_FINISHPAGE_RUN "\$INSTDIR\\StudWorx.exe"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "StudWorx"
	SetOutPath "\$INSTDIR"
	File /r "${directory_package##*/}\\*.*"
SectionEnd
EOF

cp "${directory_source}/resources/packaging/windows/setup.ico" "${directory_build}/"

# build the self extractor
(cd "${directory_build}" && "/c/Program Files (x86)/NSIS/makensis.exe" -V4 "${file_script##*/}")

printf "%s\n" "${file_package}"
