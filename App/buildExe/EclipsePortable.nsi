;Copyright (C) 2004-2012 John T. Haller
;Copyright (C) 2008-2012 Brandon Cheng (gluxon)

;Website: http://PortableApps.com/EclipsePortable

;This software is OSI Certified Open Source Software.
;OSI Certified is a certification mark of the Open Source Initiative.

;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either version 2
;of the License, or (at your option) any later version.

;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.

;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

!define PORTABLEAPPNAME "MSPSIM Portable"
!define NAME "MSPSIMPortable"
!define APPNAME "MSPSIM"
!define VER "1.0.0.0"
!define WEBSITE "PortableApps.com/EclipsePortable"
!define DEFAULTEXE "Code.exe"
!define DEFAULTAPPDIR "App\VSCode"
!define DEFAULTSETTINGSDIR "Data\settings"
!define LAUNCHERLANGUAGE "English"

;=== Program Details
Name "${PORTABLEAPPNAME}"
OutFile "..\..\${NAME}.exe"
Caption "${PORTABLEAPPNAME} | PortableApps.com"
VIProductVersion "${VER}"
VIAddVersionKey ProductName "${PORTABLEAPPNAME}"
VIAddVersionKey Comments "Allows ${APPNAME} to be run from a removable drive.  For additional details, visit ${WEBSITE}"
VIAddVersionKey CompanyName "PortableApps.com"
VIAddVersionKey LegalCopyright "PortableApps.com & Contributors"
VIAddVersionKey FileDescription "${PORTABLEAPPNAME}"
VIAddVersionKey FileVersion "${VER}"
VIAddVersionKey ProductVersion "${VER}"
VIAddVersionKey InternalName "${PORTABLEAPPNAME}"
VIAddVersionKey LegalTrademarks "PortableApps.com is a Trademark of Rare Ideas, LLC."
VIAddVersionKey OriginalFilename "${NAME}.exe"
;VIAddVersionKey PrivateBuild ""
;VIAddVersionKey SpecialBuild ""

;=== Runtime Switches
CRCCheck On
WindowIcon Off
SilentInstall Silent
AutoCloseWindow True
RequestExecutionLevel user

;=== Best Compression
SetCompress Auto
SetCompressor /SOLID lzma
SetCompressorDictSize 32
SetDatablockOptimize On

;=== Program Icon
Icon "microchip.ico"

;=== Include
;(Standard NSIS)
!include FileFunc.nsh
!include WordFunc.nsh
!include LogicLib.nsh


;(Macros)
!insertmacro WordReplace
!insertmacro GetParent
!insertmacro GetParameters
!insertmacro GetRoot

;(Custom)
!include ReadINIStrWithDefault.nsh


Var PROGRAMDIRECTORY
Var PROGRAMEXECUTABLE
Var SETTINGSDIRECTORY

Var ROOT
Var ROOTDOUBLESLASH
Var PARENTDIRECTORY
Var PARAMETERS
Var LASTEXEDIR

Section Main
	;Finding out the drive's letter and parent dir
	${GetParent} "$EXEDIR" "$PARENTDIRECTORY"
	${GetRoot} "$EXEDIR" "$ROOT"
	${GetParameters} "$PARAMETERS"
	${WordReplace} "$ROOT" ":" "\:" "+" "$ROOTDOUBLESLASH"

	;Reading the INI
	${ReadINIStrWithDefault} "$PROGRAMEXECUTABLE" "$EXEDIR\${NAME}.ini" "${NAME}" "${APPNAME}Executable" "${DEFAULTEXE}"  
	${ReadINIStrWithDefault} "$0" "$EXEDIR\${NAME}.ini" "${NAME}" "${APPNAME}Directory" "${DEFAULTAPPDIR}"
	StrCpy "$PROGRAMDIRECTORY" "$EXEDIR\$0"

	${ReadINIStrWithDefault} "$0" "$EXEDIR\${NAME}.ini" "${NAME}" "SettingsDirectory" "${DEFAULTSETTINGSDIR}"
	StrCpy "$SETTINGSDIRECTORY" "$EXEDIR\$0"

	ReadINIStr "$LASTEXEDIR" "$EXEDIR\${NAME}.ini" "${NAME}Settings" "LastEXEDirectory"
	WriteINIStr "$EXEDIR\${NAME}.ini" "${NAME}Settings" "LastEXEDirectory" "$EXEDIR"


	StrLen $0 "$PROGRAMDIRECTORY"
	${For} $1 0 $0-1
		StrCpy $2 "$PROGRAMDIRECTORY" 1 $1
		StrCmp $2 " " 0 notequal
			MessageBox MB_OK|MB_ICONEXCLAMATION "Leerzeichen in Pfad sind nicht zulässig^!$\r$\nAusführung wird abgebrochen.$\r$\nFehlerhafter Pfad$\r$\n$PROGRAMDIRECTORY$2"		
			Abort
		notequal:
	${Next}


	${If} "$LASTEXEDIR" != "$EXEDIR"
		;MessageBox MB_OK|MB_ICONEXCLAMATION "$LASTEXEDIR $EXEDIR"	   
	${EndIf}
  
  System::Call 'Kernel32::SetEnvironmentVariable(t "VSCODE_EXTENSIONS", t "") i'

	EXEC '"$PROGRAMDIRECTORY\$PROGRAMEXECUTABLE" ./Data'
	SectionEnd