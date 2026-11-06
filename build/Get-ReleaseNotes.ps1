#Requires -Version 7.2
<#
  Prints a version's release notes: its section of CHANGELOG.md, without the heading. The build workflow runs it for a
  release tag before building, so a tag without notes fails early, and the release job posts the notes it wrote.
  In Actions it also sets the step output start-tag: where the version's CHANGELOG.md compare link starts, when that is
  named like a release tag (empty when the link starts from a commit or is missing). The release job starts GitHub's
  generated list of pull requests there once it has checked that the tag exists.

    pwsh -File build\Get-ReleaseNotes.ps1 -Version 1.6.5                       print them
    pwsh -File build\Get-ReleaseNotes.ps1 -Version v1.6.5 -OutFile notes.md    write them (UTF-8)
    pwsh -File build\Get-ReleaseNotes.ps1 -Version 1.7.0-rc.2                  a release candidate's
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
# Get-Content -Raw outputs nothing for an empty file, which even a [string] cast leaves $null (unlike [string]$null).
$text = Get-Content -LiteralPath $Changelog -Raw -Encoding utf8
if ($null -eq $text) { $text = '' }
$notes = Get-ChangelogSection -Text $text -Name $Version
if ($null -eq $notes) {
  $cause = if (Test-ChangelogHeadingHidden $text $Version) { ' (its heading is inside an HTML comment or a code fence)' } else { '' }
  Stop-Notes "CHANGELOG.md has no [$Version] section${cause}: build\Release-D2BS.ps1 dates it at the release"
}
# A block that opens in the section and never closes takes the rest of the file into the notes.
if (Test-ChangelogUnclosed $notes) { Stop-Notes "an HTML comment or a code fence in CHANGELOG.md's [$Version] section never closes, so its notes would run to the end of the file" }
if (-not (Test-ChangelogEntries $notes)) { Stop-Notes "CHANGELOG.md's [$Version] section has no entries" }
foreach ($line in Get-ChangelogWrappedLines $notes) {
  $warning = "a wrapped line shows as its own line in the release notes; join it to its entry: $($line.Trim())"
  if ($actions) { Write-Host "::warning::$warning" } else { Write-Warning $warning }
}
# GitHub doesn't document which tag its generated notes start from when given none, so the release job passes this.
if ($actions -and $env:GITHUB_OUTPUT) {
  $base = Get-ChangelogCompareBase -Text $text -Name $Version
  $start = if (Test-ChangelogTagName $base) { $base } else { '' }
  Add-Content -LiteralPath $env:GITHUB_OUTPUT -Value "start-tag=$start" -Encoding utf8
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
