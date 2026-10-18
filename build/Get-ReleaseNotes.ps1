#Requires -Version 7.2
<#
  Prints a version's release notes: its section of CHANGELOG.md, without the heading. The build workflow runs it for a
  release tag before building, so a tag without notes fails early, and the release job posts the notes it wrote.

    pwsh -File build\Get-ReleaseNotes.ps1 -Version 1.6.5                       print them
    pwsh -File build\Get-ReleaseNotes.ps1 -Version v1.6.5 -OutFile notes.md    write them (UTF-8)
#>
[CmdletBinding()]
param(
  [Parameter(Mandatory)][string]$Version,
  [string]$OutFile,
  [string]$Changelog = (Join-Path (Split-Path -Parent $PSScriptRoot) 'CHANGELOG.md')
)
Set-StrictMode -Version 3
$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'Changelog.psm1') -Force

# In Actions, an annotation on the run's summary page; elsewhere a plain error or warning.
$actions = $env:GITHUB_ACTIONS -eq 'true'
function Stop-Notes([string]$Message) {
  if ($actions) { Write-Host "::error::$Message" } else { [Console]::Error.WriteLine($Message) }
  exit 1
}

$Version = $Version -replace '^v', ''
$Changelog = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Changelog)
if (-not (Test-Path -LiteralPath $Changelog -PathType Leaf)) { Stop-Notes "$Changelog does not exist" }
# Get-Content -Raw returns $null for an empty file, and a [string] cast keeps it null.
$text = Get-Content -LiteralPath $Changelog -Raw -Encoding utf8
$notes = Get-ChangelogSection -Text $(if ($null -eq $text) { '' } else { $text }) -Name $Version
if ($null -eq $notes) { Stop-Notes "CHANGELOG.md has no [$Version] section: build\Release-D2BS.ps1 dates it at the release" }
if (-not (Test-ChangelogEntries $notes)) { Stop-Notes "CHANGELOG.md's [$Version] section has no entries" }
foreach ($line in Get-ChangelogWrappedLines $notes) {
  $warning = "a wrapped line shows as its own line in the release notes; join it to its entry: $($line.Trim())"
  if ($actions) { Write-Host "::warning::$warning" } else { Write-Warning $warning }
}

if ($OutFile) {
  # .NET resolves a relative path against the process's directory, not PowerShell's.
  $OutFile = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutFile)
  $dir = Split-Path -Parent $OutFile
  if ($dir) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
  [IO.File]::WriteAllText($OutFile, "$notes`n", [Text.UTF8Encoding]::new($false))
} else {
  $notes
}
