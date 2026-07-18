#******************************************************************************
# **  @file   sd.ps1
# **  @author slspencer
# **  @date   July 17, 2026
# **
# **  @brief
# **  "seamly2d debug" — local debug-build script for seamly2d, mirroring
# **  seamlyLayout's qd.ps1 precedent. Auto-locates the newest Qt 6.10.x
# **  msvc2022_64 kit under C:\Qt and the VS 18 Community MSVC environment,
# **  then shadow-builds Seamly2D.pro with CONFIG+=debug into
# **  seamly2d-build-debug\ at the repo root (kept separate from the
# **  release build\ tree).
# **
# **  @copyright
# **  This source code is part of the Seamly2D project, a pattern making
# **  program, whose allow create and modeling patterns of clothing.
# **  Copyright (C) 2026 Seamly2D Project
# **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
# **
# **  Seamly2D is free software: you can redistribute it and/or modify
# **  it under the terms of the GNU General Public License as published by
# **  the Free Software Foundation, either version 3 of the License, or
# **  (at your option) any later version.
# **
# **  Seamly2D is distributed in the hope that it will be useful,
# **  but WITHOUT ANY WARRANTY; without even the implied warranty of
# **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# **  GNU General Public License for more details.
# **
# **  You should have received a copy of the GNU General Public License
# **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
# **
#******************************************************************************

<#
.SYNOPSIS
    Build a debug seamly2d.exe locally (Qt 6.10.x + VS 18 Community).

.DESCRIPTION
    Shadow-builds the whole Seamly2D qmake project with CONFIG+=debug into
    <repo-root>\seamly2d-build-debug\ (gitignored via the *-build-* pattern),
    keeping it separate from the release build\ tree.

    Toolchain is auto-detected and the script fails early with a clear
    message naming whatever is missing:
      * qmake  — newest C:\Qt\6.10.x\msvc2022_64 kit
      * MSVC   — VS 18 Community vcvars64.bat (its vswhere warning is
                 harmless; the script suppresses vcvars output entirely)
      * make   — C:\Qt\Tools\QtCreator\bin\jom\jom.exe (parallel), falling
                 back to nmake from the MSVC environment if jom is absent

    The seamly2d.pro post-link step runs windeployqt, so the Qt debug DLLs
    are deployed beside the executable automatically. The debug binary lands
    at seamly2d-build-debug\src\app\seamly2d\bin\seamly2d.exe.

.PARAMETER Run
    Launch the freshly built debug seamly2d.exe after a successful build.

.EXAMPLE
    .\scripts\sd.ps1
    Build (or incrementally rebuild) the debug executable.

.EXAMPLE
    .\scripts\sd.ps1 -Run
    Build, then launch the debug executable.

.NOTES
    "sd" = seamly2d debug, mirroring seamlyLayout's qd.ps1 ("Qt debug").
#>

param(
    # When set, launch the debug seamly2d.exe after a successful build.
    [switch]$Run
)

# Stop on any PowerShell-level error; native tool failures are checked via
# exit codes below.
$ErrorActionPreference = 'Stop'

#------------------------------------------------------------------------------
# @brief  Locate qmake.exe from the newest Qt 6.10.x msvc2022_64 kit.
#
# Scans C:\Qt for version directories matching 6.10.<patch>, sorts them as
# versions (so 6.10.10 beats 6.10.9), and returns the qmake.exe of the newest
# kit that actually contains msvc2022_64\bin\qmake.exe.
#
# @return Full path to qmake.exe.
#------------------------------------------------------------------------------
function Find-QtQmake {
    $qtRoot = 'C:\Qt'
    if (-not (Test-Path $qtRoot)) {
        throw "Qt root '$qtRoot' not found - install Qt 6.10.x (msvc2022_64) first."
    }

    # Collect 6.10.x kit dirs that ship the MSVC 2022 64-bit qmake,
    # newest patch version first.
    $kits = Get-ChildItem $qtRoot -Directory |
        Where-Object { $_.Name -match '^6\.10\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending

    foreach ($kit in $kits) {
        $qmake = Join-Path $kit.FullName 'msvc2022_64\bin\qmake.exe'
        if (Test-Path $qmake) { return $qmake }
    }

    throw "No Qt 6.10.x kit with msvc2022_64\bin\qmake.exe found under '$qtRoot'."
}

#------------------------------------------------------------------------------
# @brief  Locate the VS 18 Community 64-bit MSVC environment script.
#
# @return Full path to vcvars64.bat.
#------------------------------------------------------------------------------
function Find-VcVars64 {
    $vcvars = 'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) {
        throw "VS 18 Community vcvars64.bat not found at '$vcvars' - install Visual Studio 18 Community with the C++ workload."
    }
    return $vcvars
}

#------------------------------------------------------------------------------
# @brief  Pick the make tool: jom (parallel) if present, else nmake.
#
# nmake needs no path - it is on PATH once vcvars64.bat has run inside the
# generated batch file below.
#
# @return Full path to jom.exe, or the bare string 'nmake' as fallback.
#------------------------------------------------------------------------------
function Find-MakeTool {
    $jom = 'C:\Qt\Tools\QtCreator\bin\jom\jom.exe'
    if (Test-Path $jom) { return $jom }
    Write-Host "jom not found at '$jom' - falling back to nmake (single-threaded)."
    return 'nmake'
}

# --- Resolve toolchain and paths (fail early with clear messages) ------------
$qmake    = Find-QtQmake
$vcvars   = Find-VcVars64
$makeTool = Find-MakeTool

# The script lives in <repo-root>\scripts\, so the repo root is its parent.
$repoRoot = Split-Path -Parent $PSScriptRoot
$proFile  = Join-Path $repoRoot 'Seamly2D.pro'
if (-not (Test-Path $proFile)) {
    throw "Seamly2D.pro not found at '$proFile' - is the script still in <repo-root>\scripts\?"
}

# Dedicated shadow-build dir, separate from the release build\ tree.
# The name matches the *-build-* .gitignore pattern, so it is never committed.
$buildDir = Join-Path $repoRoot 'seamly2d-build-debug'
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "qmake : $qmake"
Write-Host "msvc  : $vcvars"
Write-Host "make  : $makeTool"
Write-Host "build : $buildDir"

# --- Generate and run the build batch file -----------------------------------
# vcvars64.bat must be 'call'ed from a batch context to import the MSVC
# environment, so the whole qmake+make sequence runs inside one cmd.exe via a
# generated .cmd file (this also sidesteps cmd/PowerShell quoting pitfalls).
# vcvars output is discarded: it only prints a banner plus a harmless vswhere
# warning, and a failure is still caught through its exit code.
$batch = Join-Path $buildDir 'sd-build.cmd'
@"
@echo off
call "$vcvars" >nul 2>&1
if errorlevel 1 echo vcvars64.bat failed & exit /b 1
"$qmake" "$proFile" CONFIG+=debug
if errorlevel 1 exit /b 1
"$makeTool"
"@ | Set-Content -Path $batch -Encoding Ascii

# Run the batch with the shadow-build dir as working directory so qmake
# writes all Makefiles and objects there instead of into the source tree.
Push-Location $buildDir
try {
    & cmd.exe /d /c $batch
    $buildExit = $LASTEXITCODE
}
finally {
    Pop-Location
}
if ($buildExit -ne 0) {
    throw "Build failed (exit code $buildExit) - see output above."
}

# --- Verify the result --------------------------------------------------------
# seamly2d.pro sets DESTDIR = bin and post-links windeployqt, so the debug
# exe and its Qt debug DLLs (Qt6Cored.dll etc.) must both be present.
$exe = Join-Path $buildDir 'src\app\seamly2d\bin\seamly2d.exe'
if (-not (Test-Path $exe)) {
    throw "Build reported success but '$exe' is missing."
}
$coreDll = Join-Path (Split-Path $exe) 'Qt6Cored.dll'
if (-not (Test-Path $coreDll)) {
    throw "Qt debug DLLs were not deployed beside '$exe' (Qt6Cored.dll missing) - check the windeployqt post-link step output."
}

Write-Host ''
Write-Host "Debug build OK: $exe"

# Optionally launch the freshly built debug executable (detached, so the
# console is not blocked by the GUI app).
if ($Run) {
    Write-Host 'Launching debug seamly2d.exe ...'
    Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe)
}
