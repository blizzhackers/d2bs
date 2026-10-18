# CHANGELOG.md (Keep a Changelog) for build\Release-D2BS.ps1, which dates the Unreleased section at a release, and
# build\Get-ReleaseNotes.ps1, which hands a version's section to the release build as its GitHub release notes.
Set-StrictMode -Version 3

function Split-ChangelogLines([AllowEmptyString()][string]$Text) { , @($Text -split '\r?\n') }

# The line index of "## [<Name>]" (with or without " - <date>"), or -1.
function Find-ChangelogHeading([string[]]$Lines, [string]$Name) {
  $pattern = '^## \[' + [regex]::Escape($Name) + '\]( |$)'
  for ($i = 0; $i -lt $Lines.Count; $i++) { if ($Lines[$i] -cmatch $pattern) { return $i } }
  -1
}

# The text under a section's heading, without its surrounding blank lines: '' when empty, $null when the heading is
# missing. A section ends at the next "## " heading or at the version links closing the file.
function Get-ChangelogSection {
  param([Parameter(Mandatory)][AllowEmptyString()][string]$Text, [Parameter(Mandatory)][string]$Name)
  $lines = Split-ChangelogLines $Text
  $start = Find-ChangelogHeading $lines $Name
  if ($start -lt 0) { return $null }
  $end = $start + 1
  while ($end -lt $lines.Count -and $lines[$end] -notmatch '^## ' -and $lines[$end] -cnotmatch '^\[(Unreleased|\d+\.\d+\.\d+)\]: ') { $end++ }
  $first = $start + 1
  $last = $end - 1
  while ($first -le $last -and -not $lines[$first].Trim()) { $first++ }
  while ($last -ge $first -and -not $lines[$last].Trim()) { $last-- }
  # A PowerShell range runs backwards when its end is lower, so an empty section needs its own case.
  if ($first -gt $last) { return '' }
  @($lines[$first..$last]) -join "`n"
}

# Whether a section holds at least one entry (a list item): group headings alone make no release notes.
function Test-ChangelogEntries([AllowEmptyString()][string]$Section) { [bool]($Section -cmatch '(?m)^\s*([-*+]|\d+[.)])\s') }

# Lines that continue a wrapped bullet. GitHub shows every newline of a release body as a line break, so each entry
# stays on one line; nested list items are fine.
function Get-ChangelogWrappedLines([AllowEmptyString()][string]$Section) {
  @((Split-ChangelogLines $Section) | Where-Object { $_ -match '^\s+\S' -and $_ -notmatch '^\s+([-*+]|\d+[.)])\s' })
}

# Dates the Unreleased section as "## [<Version>] - <Date>" under a new, empty Unreleased heading, and moves the
# "[Unreleased]: <repo>/compare/<base>...HEAD" link to <Tag>, adding "[<Version>]: <repo>/compare/<base>...<Tag>".
# Keeps the file's line endings. Throws when there is nothing to release.
function Update-ChangelogForRelease {
  param(
    [Parameter(Mandatory)][AllowEmptyString()][string]$Text,
    [Parameter(Mandatory)][string]$Version,
    [Parameter(Mandatory)][string]$Date,
    [Parameter(Mandatory)][string]$Tag
  )
  $newline = if ($Text.Contains("`r`n")) { "`r`n" } else { "`n" }
  $lines = [Collections.Generic.List[string]]::new([string[]](Split-ChangelogLines $Text))
  $unreleased = Find-ChangelogHeading $lines 'Unreleased'
  if ($unreleased -lt 0) { throw 'CHANGELOG.md has no "## [Unreleased]" heading' }
  if ((Find-ChangelogHeading $lines $Version) -ge 0) { throw "CHANGELOG.md already has a [$Version] section" }
  if (-not (Test-ChangelogEntries (Get-ChangelogSection -Text $Text -Name 'Unreleased'))) {
    throw 'CHANGELOG.md has no entries under [Unreleased]: the release would have no notes'
  }
  $lines.Insert($unreleased + 1, '')
  $lines.Insert($unreleased + 2, "## [$Version] - $Date")

  $link = -1
  for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -cmatch '^\[Unreleased\]: ') { $link = $i; break } }
  if ($link -ge 0) {
    if ($lines[$link] -cnotmatch '^\[Unreleased\]: (\S+)/compare/(\S+)\.\.\.HEAD$') {
      throw "CHANGELOG.md's [Unreleased] link is not <repo>/compare/<base>...HEAD: $($lines[$link])"
    }
    $repoUrl = $Matches[1]
    $base = $Matches[2]
    $lines[$link] = "[Unreleased]: $repoUrl/compare/$Tag...HEAD"
    $lines.Insert($link + 1, "[$Version]: $repoUrl/compare/$base...$Tag")
  }
  $lines -join $newline
}

Export-ModuleMember -Function Get-ChangelogSection, Test-ChangelogEntries, Get-ChangelogWrappedLines, Update-ChangelogForRelease
