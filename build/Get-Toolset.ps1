#Requires -Version 7.2
<#
  Fetches the Visual C++ 2010 SP1 x86 compiler and the Windows SDK 7.1 headers and libraries listed in toolset.json
  from Microsoft, checks their size and SHA-256, and unpacks them into -Out without installing anything: 7-Zip takes
  each MSI out of its download, then msiexec /a makes an administrative image (a plain extract: the only custom
  actions those MSIs run in that mode set directory properties). Only the x86 parts the D2BS build uses are kept.
  js_static.lib (SpiderMonkey, built with VS2010) refuses any newer linker, which is why this toolset is needed.

    pwsh -File build\Get-Toolset.ps1 -Out C:\d2bs-toolset
    pwsh -File build\Get-Toolset.ps1 -Out C:\d2bs-toolset -PackageDir $HOME\Downloads   # already-downloaded files
#>
param(
  [Parameter(Mandatory)][string]$Out,
  [string]$PackageDir,
  [string]$SevenZip
)
Set-StrictMode -Version 3
$ErrorActionPreference = 'Stop'
# Relative paths resolve against PowerShell's current location; .NET would use the process's own directory.
$Out = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Out).TrimEnd('\', '/')
if ($Out -match '^[A-Za-z]:$') { throw "-Out must be a folder, not the drive root $Out" }
if ($PackageDir) { $PackageDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($PackageDir) }
# -Out is replaced whole, so it must be new, empty, or a toolset this script made (it holds toolset.txt).
if ((Test-Path -LiteralPath $Out) -and @(Get-ChildItem -LiteralPath $Out -Force).Count -and
    -not (Test-Path -LiteralPath (Join-Path $Out 'toolset.txt'))) {
  throw "$Out already holds files and is not a toolset made by this script (no toolset.txt): pass a new or empty folder"
}
$work = Join-Path ([IO.Path]::GetTempPath()) "d2bs-toolset-$PID"
$spec = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'toolset.json') -Raw | ConvertFrom-Json
if (-not $SevenZip) {
  $onPath = Get-Command 7z.exe -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty Source
  $SevenZip = @($onPath, "$env:ProgramFiles\7-Zip\7z.exe") | Where-Object { $_ -and (Test-Path -LiteralPath $_) } |
    Select-Object -First 1
}
if (-not $SevenZip) { throw 'needs 7-Zip: put 7z.exe on PATH or pass -SevenZip' }

function Get-Package($pkg) {
  $local = if ($PackageDir) { Join-Path $PackageDir $pkg.file } else { $null }
  $path = if ($local -and (Test-Path -LiteralPath $local)) { $local } else {
    $dest = Join-Path $work $pkg.file
    Write-Host "downloading $($pkg.url)"
    # A reset or a stall partway through the 600 MB image is retried too; the size and hash check catch the rest.
    & curl.exe --fail --location --silent --show-error --retry 3 --retry-all-errors --connect-timeout 30 `
      --speed-limit 102400 --speed-time 60 --output $dest $pkg.url
    if ($LASTEXITCODE) { throw "download of $($pkg.file) failed (curl $LASTEXITCODE)" }
    $dest
  }
  $size = (Get-Item -LiteralPath $path).Length
  $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
  if ($size -ne $pkg.size -or $hash -ne $pkg.sha256) {
    throw "$($pkg.file): $size bytes, sha256 $hash; expected $($pkg.size) bytes, sha256 $($pkg.sha256)"
  }
  $path
}

function Expand-Msi($pkg, [string]$archive) {
  $dir = Join-Path $work $pkg.name
  & $SevenZip e $archive "-o$dir" -y @($pkg.extract) | Out-Null
  if ($LASTEXITCODE) { throw "7-Zip could not unpack $($pkg.file) ($LASTEXITCODE)" }
  $msi = Join-Path $dir $pkg.msi
  if (-not (Test-Path -LiteralPath $msi)) { throw "$($pkg.file) holds no $($pkg.msi)" }
  $image = Join-Path $work "$($pkg.name)-image"
  $log = Join-Path $work "$($pkg.name)-msiexec.log"
  $p = Start-Process -FilePath msiexec.exe -Wait -PassThru -ArgumentList @('/a', "`"$msi`"", '/qn', "TARGETDIR=`"$image`"", '/L*', "`"$log`"")
  if ($p.ExitCode -notin 0, 3010) {
    # The log goes with the work folder, so show its errors (the failing action ends with "Return value 3") and end.
    if (Test-Path -LiteralPath $log) {
      $lines = @(Get-Content -LiteralPath $log)
      $errors = @($lines | Where-Object { $_ -match 'Return value 3|Error \d{4}' } | Select-Object -First 10)
      if ($errors) { Write-Host 'msiexec errors:'; $errors | Write-Host }
      Write-Host 'msiexec log, last lines:'; $lines | Select-Object -Last 30 | Write-Host
    }
    throw "msiexec /a $($pkg.msi) failed ($($p.ExitCode))"
  }
  $image
}

# The single directory under $root that holds every one of $markers (relative paths), or a clear error.
function Find-Root([string]$root, [string]$leaf, [string[]]$markers) {
  $hits = @(Get-ChildItem -LiteralPath $root -Recurse -Directory -Filter $leaf | Where-Object {
    $d = $_.FullName; @($markers | Where-Object { -not (Test-Path -LiteralPath (Join-Path $d $_)) }).Count -eq 0
  })
  if ($hits.Count -ne 1) { throw "expected one '$leaf' folder with $($markers -join ', ') under $root, found $($hits.Count)" }
  $hits[0].FullName
}

# The one x86 (PE machine 0x14C) copy of $name under $root, or a clear error.
function Find-X86Dll([string]$root, [string]$name) {
  $hits = @(Get-ChildItem -LiteralPath $root -Recurse -File -Filter $name | Where-Object {
    $b = [IO.File]::ReadAllBytes($_.FullName)
    [BitConverter]::ToUInt16($b, [BitConverter]::ToInt32($b, 0x3C) + 4) -eq 0x14C
  })
  if ($hits.Count -ne 1) { throw "expected one x86 $name under $root, found $($hits.Count)" }
  $hits[0].FullName
}

# Copies the files of $from (with -Recurse, its subfolders too) into $to.
function Copy-Tree([string]$from, [string]$to, [switch]$Recurse) {
  New-Item -ItemType Directory -Force -Path $to | Out-Null
  if ($Recurse) { Get-ChildItem -LiteralPath $from -Force | Copy-Item -Destination $to -Recurse -Force }
  else { Get-ChildItem -LiteralPath $from -File | Copy-Item -Destination $to -Force }
}

# The toolset is assembled and checked beside -Out, then swapped in: a failed run leaves -Out as it was. The names
# are per run, so a folder that a killed run left behind is never built into or renamed over.
$runId = "$PID-$([guid]::NewGuid().ToString('N').Substring(0, 8))"
$stage = "$Out.partial-$runId"
New-Item -ItemType Directory -Force -Path $work | Out-Null
try {
  $images = @{}
  foreach ($pkg in $spec.packages) { $images[$pkg.name] = Expand-Msi $pkg (Get-Package $pkg) }
  $vc = Find-Root $images.vc 'VC' @('bin\cl.exe', 'include\stdio.h', 'lib\msvcrt.lib')
  $ide = Find-Root $images.vc 'IDE' @('mspdb100.dll')
  $sdk = Find-Root $images.sdk 'v7.1' @('Include\Windows.h', 'Lib\kernel32.lib')
  # x86 only: the top level of VC\bin, VC\lib and the SDK's Lib; amd64/ia64 subfolders are left out.
  Copy-Tree (Join-Path $vc 'bin') (Join-Path $stage 'vc\bin')
  Copy-Tree (Join-Path $vc 'bin\1033') (Join-Path $stage 'vc\bin\1033')
  Copy-Tree (Join-Path $vc 'include') (Join-Path $stage 'vc\include') -Recurse
  Copy-Tree (Join-Path $vc 'lib') (Join-Path $stage 'vc\lib')
  Copy-Tree $ide (Join-Path $stage 'ide')
  Copy-Tree (Join-Path $sdk 'Include') (Join-Path $stage 'sdk\include') -Recurse
  Copy-Tree (Join-Path $sdk 'Lib') (Join-Path $stage 'sdk\lib')
  # cl, link and mspdb100 load the VC++ 2010 runtime, which build machines no longer have; the MSI ships it for
  # the system folder, and Windows looks for it beside the program first.
  foreach ($name in 'msvcr100.dll', 'msvcp100.dll') {
    $dll = Find-X86Dll $images.vc $name
    foreach ($to in 'vc\bin', 'ide') { Copy-Item -LiteralPath $dll -Destination (Join-Path $stage $to) -Force }
  }
  # cl prints its banner on stderr and its usage on stdout, in no fixed order once merged. PATH is the caller's
  # again afterwards, as the script may run in-process.
  $savedPath = $env:PATH
  try {
    $env:PATH = (Join-Path $stage 'vc\bin') + ';' + (Join-Path $stage 'ide') + ';' + $env:PATH
    $clOut = @(& (Join-Path $stage 'vc\bin\cl.exe') 2>&1 | ForEach-Object { "$_" })
  } finally { $env:PATH = $savedPath }
  $banner = @($clOut -match 'Version 16\.00\.40219') | Select-Object -First 1
  if (-not $banner) { throw "unexpected compiler output: $($clOut -join ' | ')" }
  # Written last, once the toolset is known good. Build-D2BS.ps1 checks these lines against toolset.json, so bump
  # "recipe" there whenever this script changes what it puts in the toolset.
  Set-Content -LiteralPath (Join-Path $stage 'toolset.txt') -Encoding utf8 -Value @(
    "recipe $($spec.recipe)"
    ($spec.packages | ForEach-Object { "$($_.file) sha256 $($_.sha256)" })
  )
  # Swapped by renames: a rename fails whole while a file in the folder is open, where a delete stops halfway and
  # Move-Item falls back to moving file by file. A running program inside doesn't block a rename.
  $old = $null
  if (Test-Path -LiteralPath $Out) {
    # cl /Zi leaves a PDB server running from the toolset for a while after a build, holding its DLLs open.
    Get-CimInstance Win32_Process -Filter "Name='mspdbsrv.exe'" | Where-Object {
      $_.ExecutablePath -and $_.ExecutablePath.StartsWith($Out + '\', [StringComparison]::OrdinalIgnoreCase)
    } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force; Wait-Process -Id $_.ProcessId -Timeout 10 -ErrorAction SilentlyContinue }
    $old = "$Out.old-$runId"
    try { Rename-Item -LiteralPath $Out -NewName (Split-Path -Leaf $old) }
    catch { throw "$Out could not be renamed (is a build running from it?), so it was left as it was: $($_.Exception.Message)" }
  }
  try { Rename-Item -LiteralPath $stage -NewName (Split-Path -Leaf $Out) }
  catch {
    $failure = $_
    if ($old) {
      try { Rename-Item -LiteralPath $old -NewName (Split-Path -Leaf $Out) }
      catch { throw "the new toolset could not be renamed to $Out ($($failure.Exception.Message)), nor the old one back: rename $old to $Out by hand" }
    }
    throw $failure
  }
  # Old toolsets go once no program runs from them: this run's, and any an earlier run had to leave behind. Only
  # names this script makes match, so a backup someone named <Out>.old-something is never touched.
  $ours = '^' + [regex]::Escape((Split-Path -Leaf $Out)) + '\.old-\d+-[0-9a-f]{8}$'
  Get-ChildItem -LiteralPath (Split-Path -Parent $Out) -Directory | Where-Object { $_.Name -match $ours } |
    ForEach-Object { Remove-Item -LiteralPath $_.FullName -Recurse -Force -ErrorAction SilentlyContinue }
  if ($old -and (Test-Path -LiteralPath $old)) { Write-Warning "a program still uses the old toolset, so $old is left behind; a later run deletes it" }
  $mb = [math]::Round(((Get-ChildItem -LiteralPath $Out -Recurse -File | Measure-Object -Property Length -Sum).Sum) / 1MB)
  Write-Host "toolset ready in $Out ($mb MB): $banner"
} finally {
  Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
  if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue }
}
