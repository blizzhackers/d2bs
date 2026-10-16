#Requires -Version 7.2
<#
  Builds D2BS.dll (Release|Win32) with the Visual C++ 2010 SP1 toolset from Get-Toolset.ps1, without MSBuild or an
  installed Visual Studio. The flags mirror D2BS.vcxproj's Release|Win32 settings, including sqlite3.c's own (warning
  level 3, _CRT_SECURE_NO_WARNINGS). Everything the build writes goes to -Out (default build\out): objects, generated
  headers, the unpacked js_static.lib, D2BS.dll and D2BS.pdb.

  The version comes from the VERSION file (the latest release, which build\Release-D2BS.ps1 bumps) and the commit:
  "1.6.5U+dev (7c036ba)", a build after 1.6.5, and file version 1.6.5.<BuildNumber>; or with -Release -Tag v1.6.5,
  "1.6.5U (7c036ba)" and 1.6.5.0. A build with uncommitted changes to tracked files says so ("7c036ba+dirty"). The
  last file-version number is 0 only for releases: CI passes its run number, and a local build without -BuildNumber
  gets 65535. The DLL's product version string carries the full version text.

    pwsh -File build\Build-D2BS.ps1 -Toolset C:\d2bs-toolset

  PowerShell 7.2 or later: earlier versions turn a native tool's output on stderr into an error under ErrorAction
  Stop.
#>
# CmdletBinding makes an unknown argument an error: a mistyped -Release must not quietly build a dev DLL.
[CmdletBinding()]
param(
  [string]$Toolset = $env:D2BS_TOOLSET,
  [string]$Out,
  [switch]$Release,
  [string]$Tag,
  [ValidateRange(1, 65535)][int]$BuildNumber = 65535
)
Set-StrictMode -Version 3
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $Out) { $Out = Join-Path $PSScriptRoot 'out' }
if (-not $Toolset) { throw 'pass -Toolset (a folder made by build\Get-Toolset.ps1) or set D2BS_TOOLSET' }
# Relative paths resolve against PowerShell's current location; .NET would use the process's own directory.
$Out = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Out)
$Toolset = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Toolset)
foreach ($need in 'vc\bin\cl.exe', 'ide\mspdb100.dll', 'sdk\include\Windows.h', 'sdk\lib\kernel32.lib', 'toolset.txt') {
  if (-not (Test-Path -LiteralPath (Join-Path $Toolset $need))) { throw "$Toolset has no ${need}; make it with build\Get-Toolset.ps1" }
}
# A toolset made from other packages or by an older Get-Toolset.ps1 (another recipe) may lack what this build needs:
# its toolset.txt must name the recipe and the packages toolset.json names now.
$spec = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'toolset.json') -Raw | ConvertFrom-Json
$wanted = @("recipe $($spec.recipe)") + @($spec.packages | ForEach-Object { "$($_.file) sha256 $($_.sha256)" })
$made = @(Get-Content -LiteralPath (Join-Path $Toolset 'toolset.txt') | ForEach-Object { $_.Trim() } | Where-Object { $_ })
if (($made -join "`n") -ne ($wanted -join "`n")) {
  $odd = @($made | Where-Object { $_ -notin $wanted }) + @($wanted | Where-Object { $_ -notin $made } | ForEach-Object { "no $_" })
  $first = if ($odd) { $odd[0] } else { 'lines out of order or repeated' }
  throw "$Toolset was made by another toolset recipe or from other packages ($first): run build\Get-Toolset.ps1 again"
}

# Version: VERSION holds major.minor.patch; the U suffix is D2BS's (Unicode build) since 1.6.4U.
$version = (Get-Content -LiteralPath (Join-Path $repo 'VERSION') -Raw).Trim()
if ($version -notmatch '^(0|[1-9]\d{0,4})\.(0|[1-9]\d{0,4})\.(0|[1-9]\d{0,4})$') {
  throw "VERSION must be major.minor.patch without leading zeros, not '$version'"
}
$numbers = @([int]$Matches[1], [int]$Matches[2], [int]$Matches[3])
if (@($numbers | Where-Object { $_ -gt 65535 }).Count) { throw "VERSION $version does not fit a file version: each number is at most 65535" }
if ($Release -and $Tag -ne "v$version") { throw "release tag '$Tag' does not match VERSION $version (expected v$version)" }
$commit = (git -C $repo rev-parse --short=7 HEAD 2>$null)
if ($LASTEXITCODE -or -not $commit) { $commit = 'nogit' }
# Untracked files (e.g. a Visual Studio build's Release\ folder) never reach this build: only tracked changes count.
elseif (git -C $repo status --porcelain --untracked-files=no 2>$null) { $commit += '+dirty' }
$display = if ($Release) { "${version}U ($commit)" } else { "${version}U+dev ($commit)" }
$fileVersion = $numbers + @($(if ($Release) { 0 } else { $BuildNumber }))

$obj = Join-Path $Out 'obj'
$gen = Join-Path $Out 'gen'
$lib = Join-Path $Out 'lib'
New-Item -ItemType Directory -Force -Path $obj, $gen, $lib | Out-Null
$log = Join-Path $Out 'build.log'
Set-Content -LiteralPath $log -Value "D2BS build $(Get-Date -Format o): $display, file version $($fileVersion -join '.')" -Encoding utf8

# d2bs_build.h: force-included into every compile, and included by version.rc2 (D2BS_BUILD_INFO).
Set-Content -LiteralPath (Join-Path $gen 'd2bs_build.h') -Encoding ascii -Value @(
  '#ifndef D2BS_BUILD_H'
  '#define D2BS_BUILD_H'
  "#define D2BS_VERSION L`"$display`""
  "#define D2BS_FILEVERSION $($fileVersion -join ',')"
  "#define D2BS_FILEVERSION_STR `"$($fileVersion -join '.')`""
  "#define D2BS_PRODUCTVERSION_STR `"$display`""
  '#endif'
)
# The SP1 compiler update's intrin.h includes ammintrin.h but does not ship it; D2BS uses none of its intrinsics.
Set-Content -LiteralPath (Join-Path $gen 'ammintrin.h') -Encoding ascii -Value '#pragma once'

# js_static.lib is unpacked again whenever js_static.zip changes, so a reused -Out never links a stale copy.
$jsDir = Join-Path $repo 'dependencies\libs\release'
$jsZip = Join-Path $jsDir 'js_static.zip'
$jsLib = Join-Path $lib 'js_static.lib'
$jsStamp = Join-Path $lib 'js_static.zip.sha256'
$zipHash = (Get-FileHash -LiteralPath $jsZip -Algorithm SHA256).Hash
$unpacked = if (Test-Path -LiteralPath $jsStamp) { (Get-Content -LiteralPath $jsStamp -Raw).Trim() } else { '' }
if (-not (Test-Path -LiteralPath $jsLib) -or $unpacked -ne $zipHash) {
  Expand-Archive -LiteralPath $jsZip -DestinationPath $lib -Force
  Set-Content -LiteralPath $jsStamp -Value $zipHash -Encoding ascii
}
# The linker looks for js_static.lib's debug info (generated.pdb) beside the library.
Copy-Item -LiteralPath (Join-Path $jsDir 'generated.pdb') -Destination $lib -Force

# On a failure, the console gets the tool's first error lines (the cause; later ones are often its echoes) and its
# last lines; build.log keeps everything.
function Invoke-Tool([string]$Exe, [string[]]$ToolArgs) {
  Add-Content -LiteralPath $log -Value ("> $Exe " + ($ToolArgs -join ' '))
  $o = @(& $Exe @ToolArgs 2>&1 | ForEach-Object { "$_" })
  $o | Add-Content -LiteralPath $log
  if ($LASTEXITCODE) {
    $errors = @($o | Where-Object { $_ -match '\b(fatal )?error [A-Z]+\d+' } | Select-Object -First 10)
    if ($errors) { Write-Host 'First errors:'; $errors | Write-Host }
    Write-Host 'Last lines:'; $o | Select-Object -Last 20 | Write-Host
    throw "$Exe failed ($LASTEXITCODE); the whole output is in $log"
  }
}

# Run in-process (./build/Build-D2BS.ps1), the script must hand the caller back its own PATH, INCLUDE and LIB, which
# may be a Visual Studio prompt's: the values are put back, not cleared.
$savedEnv = [ordered]@{}
foreach ($name in 'PATH', 'INCLUDE', 'LIB', 'LIBPATH') { $savedEnv[$name] = [Environment]::GetEnvironmentVariable($name) }
try {
  # cl.exe loads mspdb100.dll from the IDE folder, so both go on PATH ahead of anything else.
  $env:PATH = (Join-Path $Toolset 'vc\bin') + ';' + (Join-Path $Toolset 'ide') + ';' + $env:PATH
  $env:INCLUDE = (Join-Path $Toolset 'vc\include') + ';' + (Join-Path $Toolset 'sdk\include') + ';' + $gen
  $env:LIB = (Join-Path $Toolset 'vc\lib') + ';' + (Join-Path $Toolset 'sdk\lib')
  $env:LIBPATH = Join-Path $Toolset 'vc\lib'

  # D2BS.vcxproj Release|Win32 ClCompile: full optimization, MultiThreadedDLL, no buffer checks, function-level
  # linking, no RTTI, no C++ exceptions, string pooling, its disabled warnings, ProgramDatabase debug info. The
  # warning level is per file: /W4, but /W3 for sqlite3.c.
  $defines = 'WIN32', 'XP_WIN', '_WINDOWS', 'NDEBUG', '_NDEBUG', 'WIN32_LEAN_AND_MEAN', 'STATIC_JS_API', '_MSVC_DEBUG',
    '_LIB', 'MOZILLA_STRICT_API', '_WINDLL', '_MBCS'
  $common = @('/nologo', '/c', '/Zi', '/Ox', '/GF', '/MD', '/GS-', '/Gy', '/GR-', '/fp:precise', '/Zc:wchar_t',
    '/Zc:forScope', '/Gd', '/wd4100', '/wd4127', '/wd4800', '/wd4480', '/wd4740', '/wd4530', "/I$repo",
    "/I$(Join-Path $repo 'dependencies\include')", "/I$gen", '/FId2bs_build.h', "/Fd$obj\vc100.pdb") +
    ($defines | ForEach-Object { "/D$_" })
  $sources = 'CommandLine.cpp', 'D2BS.cpp', 'D2Helpers.cpp', 'D2Intercepts.cpp', 'D2Handlers.cpp', 'D2NetHandlers.cpp',
    'Console.cpp', 'Events.cpp', 'Helpers.cpp', 'js32.cpp', 'JSProfile.cpp', 'JSSocket.cpp', 'Offset.cpp', 'Profile.cpp',
    'sqlite3.c', 'StackWalker.cpp', 'stringhash.cpp', 'Control.cpp', 'Core.cpp', 'dde.cpp', 'File.cpp', 'Game.cpp',
    'Hash.cpp', 'MPQStats.cpp', 'Room.cpp', 'ScreenHook.cpp', 'Unit.cpp', 'JSArea.cpp', 'JSControl.cpp', 'JSCore.cpp',
    'JSDirectory.cpp', 'JSExits.cpp', 'JSFile.cpp', 'JSFileTools.cpp', 'JSGame.cpp', 'JSGlobalClasses.cpp', 'JSHash.cpp',
    'JSMenu.cpp', 'JSParty.cpp', 'JSPresetUnit.cpp', 'JSRoom.cpp', 'JSSandbox.cpp', 'JSScreenHook.cpp', 'JSScript.cpp',
    'JSSQLite.cpp', 'JSUnit.cpp', 'AutoRoot.cpp', 'Script.cpp', 'ScriptEngine.cpp', 'Map\Diablo_II\ActMap.cpp',
    'Map\Diablo_II\LevelMap.cpp'
  $objs = @()
  foreach ($s in $sources) {
    $o = Join-Path $obj (([IO.Path]::GetFileNameWithoutExtension($s)) + '.obj')
    $extra = @($(if ($s -eq 'sqlite3.c') { '/W3' } else { '/W4' }))
    if ($s -in 'sqlite3.c', 'JSFile.cpp', 'JSFileTools.cpp') { $extra += '/D_CRT_SECURE_NO_WARNINGS' }
    $extra += if ($s -like '*.c') { '/TC' } else { '/TP' }
    Invoke-Tool 'cl.exe' ($common + $extra + @("/Fo$o", (Join-Path $repo $s)))
    $objs += $o
  }

  # The SDK 7.1 build package has no rc.exe; any Windows 10 or later SDK's writes the same .res format and reads the
  # SDK 7.1 headers from INCLUDE.
  $rc = Get-ChildItem -Path "${env:ProgramFiles(x86)}\Windows Kits\10\bin\10.*\x86\rc.exe" -ErrorAction SilentlyContinue |
    Sort-Object FullName | Select-Object -Last 1
  if (-not $rc) { throw 'no rc.exe: needs a Windows 10 or later SDK (Windows Kits\10)' }
  $res = Join-Path $obj 'version.res'
  Invoke-Tool $rc.FullName @('/nologo', '/l', '0x0407', '/d', 'NDEBUG', '/d', 'D2BS_BUILD_INFO', '/i', $gen, "/fo$res",
    (Join-Path $repo 'version.rc'))

  # Link: DLL, Windows subsystem, /OPT:REF, checksum (/RELEASE), fixed base (no /DYNAMICBASE), debug info.
  $dll = Join-Path $Out 'D2BS.dll'
  Invoke-Tool 'link.exe' (@('/nologo', '/DLL', '/SUBSYSTEM:WINDOWS', '/MACHINE:X86', '/OPT:REF', '/RELEASE',
    '/DYNAMICBASE:NO', '/DEBUG', "/PDB:$Out\D2BS.pdb", "/IMPLIB:$obj\D2BS.lib", "/OUT:$dll") + $objs + @($res,
    'kernel32.lib', 'user32.lib', 'gdi32.lib', 'shlwapi.lib', 'advapi32.lib', 'dbghelp.lib', 'Winmm.lib', $jsLib))
} finally {
  foreach ($name in $savedEnv.Keys) { [Environment]::SetEnvironmentVariable($name, $savedEnv[$name]) }
}

# A resource editor that rewrote the version block would ship 0.0.0.0 without an error; fail instead. FileVersion and
# ProductVersion are the string values; the *Part properties are the numeric FILEVERSION and PRODUCTVERSION.
$dllItem = Get-Item -LiteralPath $dll
$info = $dllItem.VersionInfo
$want = $fileVersion -join '.'
$found = [ordered]@{
  'file version number' = '{0}.{1}.{2}.{3}' -f $info.FileMajorPart, $info.FileMinorPart, $info.FileBuildPart, $info.FilePrivatePart
  'product version number' = '{0}.{1}.{2}.{3}' -f $info.ProductMajorPart, $info.ProductMinorPart, $info.ProductBuildPart, $info.ProductPrivatePart
  'file version string' = $info.FileVersion
  'product version string' = $info.ProductVersion
}
$expected = @($want, $want, $want, $display)
$bad = @(0..3 | Where-Object { @($found.Values)[$_] -ne $expected[$_] } | ForEach-Object { "$(@($found.Keys)[$_]) '$(@($found.Values)[$_])' (expected '$($expected[$_])')" })
if ($bad) { throw "D2BS.dll carries the wrong version: $($bad -join '; '). Check version.rc2." }
$warnings = @(Get-Content -LiteralPath $log | Where-Object { $_ -match ': warning [A-Z]+\d+' }).Count
$h = (Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash
Add-Content -LiteralPath $log -Value "built $dll sha256 $h, $warnings warnings"
Write-Host "built D2BS.dll $display, file version $($fileVersion -join '.'), $($dllItem.Length) bytes, sha256 $($h.Substring(0, 16)), $warnings warnings"
