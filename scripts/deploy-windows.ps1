# deploy-windows.ps1
#
# Creates a self-contained portable folder for seamly2d (MinGW / Qt 6.11 build).
# Double-click seamly2d.exe in the output folder will work without installing anything.
#
# Usage (from the repo root):
#   .\scripts\deploy-windows.ps1
#
# Optional params:
#   -BuildDir   path to shadow-build directory
#   -OutDir     where to create the portable folder
#   -QtDir      Qt 6 mingw_64 prefix
#   -MingwDir   MinGW bin dir
#   -Msys2Dir   MSYS2 mingw64 bin dir

param(
    [string]$BuildDir = "C:\Users\Pool\Mirror-Seamly-2D\build\Desktop_Qt_6_11_1_MinGW_64_bit-Debug",
    [string]$OutDir   = "C:\Users\Pool\Mirror-Seamly-2D\deploy-portable",
    [string]$QtDir    = "C:\Qt\6.11.1\mingw_64",
    [string]$MingwDir = "C:\Qt\Tools\mingw1310_64\bin",
    [string]$Msys2Dir = "C:\msys64\mingw64\bin"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Find-Dll {
    param([string]$name, [string[]]$searchDirs)
    foreach ($d in $searchDirs) {
        $p = Join-Path $d $name
        if (Test-Path $p) { return $p }
    }
    return $null
}

function Get-TransitiveDeps {
    param([string]$exe, [string[]]$searchDirs, [hashtable]$visited)
    $deps = New-Object System.Collections.Generic.List[string]
    try {
        $raw = & "$MingwDir\objdump.exe" -p $exe 2>$null
        $imports = $raw | Select-String "DLL Name:" | ForEach-Object {
            ($_.Line -replace '.*DLL Name:\s*', '').Trim()
        }
    } catch { return $deps }

    foreach ($dll in $imports) {
        $key = $dll.ToLower()
        if ($visited.ContainsKey($key)) { continue }
        $visited[$key] = $true

        $path = Find-Dll $dll $searchDirs
        if (-not $path) { continue }
        if ($path.ToLower().StartsWith($env:SystemRoot.ToLower())) { continue }

        $deps.Add($path)
        $sub = Get-TransitiveDeps $path $searchDirs $visited
        foreach ($s in $sub) { $deps.Add($s) }
    }
    return $deps
}

Write-Host "=== Seamly2D portable deploy ===" -ForegroundColor Cyan

$srcExe  = Join-Path $BuildDir "app\seamly2d\bin\seamly2d.exe"
$src2Exe = Join-Path $BuildDir "app\seamlyme\bin\seamlyme.exe"

if (-not (Test-Path $srcExe)) {
    Write-Error "seamly2d.exe not found at $srcExe - run the build first."
}

if (Test-Path $OutDir) { Remove-Item $OutDir -Recurse -Force }
New-Item -ItemType Directory -Force $OutDir | Out-Null
Write-Host "Output: $OutDir"

# 1) Copy executables
Copy-Item $srcExe $OutDir
if (Test-Path $src2Exe) { Copy-Item $src2Exe $OutDir }

# 2) Run windeployqt
$env:PATH = "$QtDir\bin;$MingwDir;$Msys2Dir;" + $env:PATH
Write-Host "Running windeployqt..." -ForegroundColor Yellow
$prevEAP = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& "$QtDir\bin\windeployqt.exe" --no-translations (Join-Path $OutDir "seamly2d.exe")
if (Test-Path $src2Exe) {
    & "$QtDir\bin\windeployqt.exe" --no-translations (Join-Path $OutDir "seamlyme.exe") 2>$null
}
$ErrorActionPreference = $prevEAP

# 3) Transitive DLL walk for Xerces 3.3 + ICU + MinGW runtime
Write-Host "Collecting transitive DLL dependencies..." -ForegroundColor Yellow

$searchDirs = @($Msys2Dir, $MingwDir, "$QtDir\bin")

$visited = @{}
Get-ChildItem $OutDir -Filter "*.dll" -Recurse | ForEach-Object { $visited[$_.Name.ToLower()] = $true }
Get-ChildItem $OutDir -Filter "*.exe" | ForEach-Object { $visited[$_.Name.ToLower()] = $true }

$allDeps = Get-TransitiveDeps (Join-Path $OutDir "seamly2d.exe") $searchDirs $visited

foreach ($dep in ($allDeps | Select-Object -Unique)) {
    $name = Split-Path $dep -Leaf
    $dest = Join-Path $OutDir $name
    if (-not (Test-Path $dest)) {
        Write-Host "  + $name" -ForegroundColor Green
        Copy-Item $dep $dest
    }
}

# 4) Ensure libxerces-c.dll is present
$xercesOut = Join-Path $OutDir "libxerces-c.dll"
if (-not (Test-Path $xercesOut)) {
    $xercesSource = Join-Path $Msys2Dir "libxerces-c-3-3.dll"
    if (Test-Path $xercesSource) {
        Write-Host "  + libxerces-c.dll" -ForegroundColor Green
        Copy-Item $xercesSource $xercesOut
    }
}

# 5) Copy data directories
foreach ($subDir in @("samples", "labels", "translations")) {
    $src = Join-Path $BuildDir "app\seamly2d\bin\$subDir"
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $OutDir $subDir) -Recurse -Force
        Write-Host "  + $subDir/" -ForegroundColor Green
    }
}

# 6) Summary
Write-Host ""
Write-Host "=== Result ===" -ForegroundColor Cyan
$exes = Get-ChildItem $OutDir -Filter "*.exe" | Select-Object Name, @{N="MB";E={[math]::Round($_.Length/1MB,1)}}
$dlls = (Get-ChildItem $OutDir -Filter "*.dll" -Recurse).Count
$totalBytes = (Get-ChildItem $OutDir -Recurse | Measure-Object Length -Sum).Sum
$totalMB = [math]::Round($totalBytes / 1MB, 0)
$exes | Format-Table -AutoSize
Write-Host "$dlls DLLs, ~${totalMB} MB total"
Write-Host ""
Write-Host "Portable folder ready: $OutDir" -ForegroundColor Green
Write-Host "Double-click seamly2d.exe to launch." -ForegroundColor Green
