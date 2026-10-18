# CHANGELOG.md (Keep a Changelog) for build\Release-D2BS.ps1, which dates the Unreleased section at a release, and
# build\Get-ReleaseNotes.ps1, which hands a version's section to the release build as its GitHub release notes.
Set-StrictMode -Version 3

function Split-ChangelogLines([AllowEmptyString()][string]$Text) { , @($Text -split '\r?\n') }

# The line index of "## [<Name>]" (with or without " - <date>"), or -1. Labels match in any case, as Markdown's do.
function Find-ChangelogHeading([string[]]$Lines, [string]$Name) {
  $pattern = '^## \[' + [regex]::Escape($Name) + '\]( |$)'
  for ($i = 0; $i -lt $Lines.Count; $i++) { if ($Lines[$i] -match $pattern) { return $i } }
  -1
}

# The text under a section's heading, without its surrounding blank lines: '' when empty, $null when the heading is
# missing. A section ends at the next "## " heading or at the next [Unreleased] or [<version>] link line.
function Get-ChangelogSection {
  param([Parameter(Mandatory)][AllowEmptyString()][string]$Text, [Parameter(Mandatory)][string]$Name)
  $lines = Split-ChangelogLines $Text
  $start = Find-ChangelogHeading $lines $Name
  if ($start -lt 0) { return $null }
  $end = $start + 1
  while ($end -lt $lines.Count -and $lines[$end] -notmatch '^## ' -and $lines[$end] -notmatch '^\[(Unreleased|\d+\.\d+\.\d+)\]: ') { $end++ }
  $first = $start + 1
  $last = $end - 1
  while ($first -le $last -and -not $lines[$first].Trim()) { $first++ }
  while ($last -ge $first -and -not $lines[$last].Trim()) { $last-- }
  # A PowerShell range runs backwards when its end is lower, so an empty section needs its own case.
  if ($first -gt $last) { return '' }
  @($lines[$first..$last]) -join "`n"
}

# A section without its HTML comments and fenced code, which GitHub doesn't show as text; one left open runs to the
# end of the section.
function Remove-ChangelogHidden([AllowEmptyString()][string]$Section) {
  $Section -replace '(?s)<!--.*?(-->|\z)', '' -replace '(?ms)^ {0,3}(```|~~~).*?(^ {0,3}\1[^\n]*$|\z)', ''
}

# A section's entries per "### " group, e.g. Added = 2: its top-level list items (no deeper than the first one),
# outside HTML comments and fenced code. Group headings alone, or a commented-out template, make no release notes,
# and a thematic break (* * *) is no item.
function Get-ChangelogEntryCounts([AllowEmptyString()][string]$Section) {
  $counts = [ordered]@{}
  $group = '(no group)'
  $top = $null
  # Split-ChangelogLines returns its array as one object: a foreach over the call itself would get it whole.
  $lines = Split-ChangelogLines (Remove-ChangelogHidden $Section)
  foreach ($line in $lines) {
    if ($line -match '^### (.+)$') { $group = $Matches[1].Trim() }
    elseif ($line -match '^( {0,3})([-*+]|\d{1,9}[.)])[ \t]+\S') {
      $indent = $Matches[1].Length
      if ($line -match '^ {0,3}([-*_])([ \t]*\1){2,}[ \t]*$') { continue }
      if ($null -eq $top) { $top = $indent }
      if ($indent -le $top) { $counts[$group] = 1 + $(if ($counts.Contains($group)) { $counts[$group] } else { 0 }) }
    }
  }
  $counts
}

function Test-ChangelogEntries([AllowEmptyString()][string]$Section) { (Get-ChangelogEntryCounts $Section).Count -gt 0 }

# Indented lines that continue a wrapped bullet, outside HTML comments and fenced code. GitHub shows every newline
# of a release body as a line break, so each entry stays on one line; nested list items are fine.
function Get-ChangelogWrappedLines([AllowEmptyString()][string]$Section) {
  @((Split-ChangelogLines (Remove-ChangelogHidden $Section)) | Where-Object { $_ -match '^\s+\S' -and $_ -notmatch '^\s+([-*+]|\d+[.)])\s' })
}

# Dates the Unreleased section as "## [<Version>] - <Date>" under a new, empty Unreleased heading, and moves the
# "[Unreleased]: <repo>/compare/<base>...HEAD" link to <Tag>, adding "[<Version>]: <repo>/compare/<base>...<Tag>".
# Writes CRLF throughout when the file has any, else LF. Throws when there is no Unreleased heading or entry, the
# version already has a section, or the Unreleased link is not a compare link.
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
  for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -match '^\[Unreleased\]: ') { $link = $i; break } }
  if ($link -ge 0) {
    if ($lines[$link] -notmatch '^\[Unreleased\]: (\S+)/compare/(\S+)\.\.\.HEAD$') {
      throw "CHANGELOG.md's [Unreleased] link is not <repo>/compare/<base>...HEAD: $($lines[$link])"
    }
    $repoUrl = $Matches[1]
    $base = $Matches[2]
    $lines[$link] = "[Unreleased]: $repoUrl/compare/$Tag...HEAD"
    $lines.Insert($link + 1, "[$Version]: $repoUrl/compare/$base...$Tag")
  }
  $lines -join $newline
}

Export-ModuleMember -Function Get-ChangelogSection, Get-ChangelogEntryCounts, Test-ChangelogEntries, Get-ChangelogWrappedLines,
  Update-ChangelogForRelease
