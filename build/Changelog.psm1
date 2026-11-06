# CHANGELOG.md (Keep a Changelog) for build\Release-D2BS.ps1, which dates the Unreleased section at a release (and at a
# final release merges the release candidates' sections into it), and build\Get-ReleaseNotes.ps1, which hands a
# version's section to the release build as its GitHub release notes.
Set-StrictMode -Version 3

# A version label: major.minor.patch, or a release candidate's major.minor.patch-rc.N.
$script:versionLabel = '[0-9]+\.[0-9]+\.[0-9]+(-rc\.[0-9]+)?'
# The order Keep a Changelog gives its groups; a merged section keeps it, with any other group after them.
$script:groupOrder = 'Added', 'Changed', 'Deprecated', 'Removed', 'Fixed', 'Security'

function Split-ChangelogLines([AllowEmptyString()][string]$Text) { , @($Text -split '\r?\n') }

# Which lines carry no Markdown structure on GitHub: an HTML comment block, which it hides (from a line opening with
# "<!--" to the line holding "-->"), and a fenced code block, which it shows as code (its fences included; it closes
# on a fence of the same character, at least as long, with nothing after it). One left open runs to the end. Every
# reader of CHANGELOG.md's structure here skips these lines, so a commented-out template is never taken for a heading,
# a link or an entry. A "<!--" later in a line opens no block: it hides at most its own paragraph's text, where no
# heading can sit.
function Find-ChangelogHiddenLines([string[]]$Lines) {
  $hidden = [bool[]]::new($Lines.Count)
  $comment = $false
  $fence = $null
  for ($i = 0; $i -lt $Lines.Count; $i++) {
    $line = $Lines[$i]
    if ($fence) {
      $hidden[$i] = $true
      if ($line -match ('^ {0,3}' + [regex]::Escape($fence) + [regex]::Escape([string]$fence[0]) + '*[ \t]*$')) { $fence = $null }
    } elseif ($comment) {
      $hidden[$i] = $true
      if ($line.Contains('-->')) { $comment = $false }
    } elseif ($line -match '^ {0,3}(`{3,})[^`]*$' -or $line -match '^ {0,3}(~{3,})') {
      # A backtick in the info string makes the line inline code, not a fence.
      $hidden[$i] = $true
      $fence = $Matches[1]
    } elseif ($line -match '^ {0,3}<!--') {
      # The "--" that opens the comment may start its "-->" too, so "<!-->" closes on its own line.
      $hidden[$i] = $true
      $comment = $line.IndexOf('-->', $line.IndexOf('<!--') + 2) -lt 0
    }
  }
  , $hidden
}

# Whether an HTML comment or a code fence never closes, so the rest of the file carries no structure.
function Test-ChangelogUnclosed([AllowEmptyString()][string]$Text) {
  $lines = [Collections.Generic.List[string]]::new([string[]](Split-ChangelogLines $Text))
  $lines.Add('end')
  (Find-ChangelogHiddenLines $lines.ToArray())[-1]
}

# The line index of "## [<Name>]" (with or without " - <date>"), or -1. Labels match in any case, as Markdown's do.
function Find-ChangelogHeading([string[]]$Lines, [string]$Name) {
  $pattern = '^## \[' + [regex]::Escape($Name) + '\]( |$)'
  $hidden = Find-ChangelogHiddenLines $Lines
  for ($i = 0; $i -lt $Lines.Count; $i++) { if (-not $hidden[$i] -and $Lines[$i] -match $pattern) { return $i } }
  -1
}

# Where the section whose heading is at $Start ends: the next "## " heading, the next [Unreleased] or [<version>] link
# line, or the end of the file.
function Find-ChangelogSectionEnd([string[]]$Lines, [int]$Start) {
  $hidden = Find-ChangelogHiddenLines $Lines
  $end = $Start + 1
  while ($end -lt $Lines.Count -and ($hidden[$end] -or ($Lines[$end] -notmatch '^## ' -and $Lines[$end] -notmatch "^\[(Unreleased|$script:versionLabel)\]: "))) { $end++ }
  $end
}

# Lines $First up to $End (exclusive) without the blank lines around them, as one text; '' when nothing is left.
function Join-ChangelogBody([string[]]$Lines, [int]$First, [int]$End) {
  $last = $End - 1
  while ($First -le $last -and -not $Lines[$First].Trim()) { $First++ }
  while ($last -ge $First -and -not $Lines[$last].Trim()) { $last-- }
  # A PowerShell range runs backwards when its end is lower, so an empty section needs its own case.
  if ($First -gt $last) { return '' }
  @($Lines[$First..$last]) -join "`n"
}

# The text under a section's heading, without its surrounding blank lines: '' when empty, $null when the heading is
# missing.
function Get-ChangelogSection {
  param([Parameter(Mandatory)][AllowEmptyString()][string]$Text, [Parameter(Mandatory)][string]$Name)
  $lines = Split-ChangelogLines $Text
  $start = Find-ChangelogHeading $lines $Name
  if ($start -lt 0) { return $null }
  Join-ChangelogBody $lines ($start + 1) (Find-ChangelogSectionEnd $lines $start)
}

# The release candidates' sections right below [Unreleased], before any final version's, newest first: the ones the
# next final release merges. Each has its Name (1.7.0-rc.2), the index of its heading (Start) and its End.
function Find-ChangelogCandidates([string[]]$Lines) {
  $found = [Collections.Generic.List[object]]::new()
  $unreleased = Find-ChangelogHeading $Lines 'Unreleased'
  if ($unreleased -lt 0) { return , $found.ToArray() }
  $at = Find-ChangelogSectionEnd $Lines $unreleased
  while ($at -lt $Lines.Count -and $Lines[$at] -match '^## \[([0-9]+\.[0-9]+\.[0-9]+-rc\.[0-9]+)\]( |$)') {
    $name = $Matches[1]
    $end = Find-ChangelogSectionEnd $Lines $at
    $found.Add([pscustomobject]@{ Name = $name; Start = $at; End = $end })
    $at = $end
  }
  , $found.ToArray()
}

# The names of the release candidates' sections the next final release merges, newest first.
function Get-ChangelogCandidates([AllowEmptyString()][string]$Text) {
  $candidates = Find-ChangelogCandidates (Split-ChangelogLines $Text)
  , @(foreach ($c in $candidates) { $c.Name })
}

# The target of a "[<Name>]: <target>" link line outside hidden text, or $null. Labels match in any case.
function Get-ChangelogLink([AllowEmptyString()][string]$Text, [string]$Name) {
  $pattern = '^\[' + [regex]::Escape($Name) + '\]: (\S+)'
  $lines = Split-ChangelogLines $Text
  $hidden = Find-ChangelogHiddenLines $lines
  for ($i = 0; $i -lt $lines.Count; $i++) { if (-not $hidden[$i] -and $lines[$i] -match $pattern) { return $Matches[1] } }
  $null
}

# The <base> of a version's "[<Name>]: <repo>/compare/<base>...<tag>" link, or $null.
function Get-ChangelogCompareBase([AllowEmptyString()][string]$Text, [string]$Name) {
  $target = Get-ChangelogLink $Text $Name
  if ($target -match '/compare/(\S+?)\.\.\.\S+$') { $Matches[1] } else { $null }
}

# Chunks of section text joined in order. A chunk that opens with a bullet or with "1." goes on the next line, so the
# lists of several sections stay one list (Markdown lets only those start a list right after a paragraph); any other
# starts after a blank line, so a paragraph never joins the item before it.
function Join-ChangelogChunks([string[]]$Chunks) {
  $text = ''
  foreach ($chunk in $Chunks) {
    if (-not $text) { $text = $chunk }
    elseif ($chunk -match '^ {0,3}([-*+]|1[.)])[ \t]') { $text += "`n$chunk" }
    else { $text += "`n`n$chunk" }
  }
  $text
}

# Section bodies merged into one, oldest first: their text before any "### " group, then each group once, holding the
# group's lists from every body in order. Groups left empty drop out.
function Merge-ChangelogBodies([string[]]$Bodies) {
  $segments = [Collections.Generic.List[object]]::new()
  foreach ($body in $Bodies) {
    $lines = Split-ChangelogLines $body
    $hidden = Find-ChangelogHiddenLines $lines
    $group = $null
    $from = 0
    for ($i = 0; $i -le $lines.Count; $i++) {
      if ($i -eq $lines.Count -or (-not $hidden[$i] -and $lines[$i] -match '^### (.+)$')) {
        $text = Join-ChangelogBody $lines $from $i
        if ($text) { $segments.Add([pscustomobject]@{ Group = $group; Text = $text }) }
        if ($i -lt $lines.Count) { $group = $Matches[1].Trim(); $from = $i + 1 }
      }
    }
  }
  # Group names match in any case; the first spelling met is the one kept.
  $names = [Collections.Generic.List[string]]::new()
  foreach ($s in $segments) { if ($null -ne $s.Group -and $s.Group -notin $names) { $names.Add($s.Group) } }
  $ordered = @(foreach ($g in $script:groupOrder) { $names | Where-Object { $_ -eq $g } }) + @($names | Where-Object { $_ -notin $script:groupOrder })
  $parts = [Collections.Generic.List[string]]::new()
  $intro = Join-ChangelogChunks @($segments | Where-Object { $null -eq $_.Group } | ForEach-Object { $_.Text })
  if ($intro) { $parts.Add($intro) }
  foreach ($g in $ordered) {
    $parts.Add("### $g`n`n" + (Join-ChangelogChunks @($segments | Where-Object { $null -ne $_.Group -and $_.Group -eq $g } | ForEach-Object { $_.Text })))
  }
  $parts -join "`n`n"
}

# A section without its HTML comment blocks and fenced code (see Find-ChangelogHiddenLines).
function Remove-ChangelogHidden([AllowEmptyString()][string]$Section) {
  $lines = Split-ChangelogLines $Section
  $hidden = Find-ChangelogHiddenLines $lines
  @(for ($i = 0; $i -lt $lines.Count; $i++) { if (-not $hidden[$i]) { $lines[$i] } }) -join "`n"
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
# A final version (no -rc.N) also merges the release candidates' sections below Unreleased into its own (oldest
# first, see Merge-ChangelogBodies) and drops them and their links; its link then starts where the oldest candidate's
# did, and without that candidate's link it gets none. Writes CRLF throughout when the file has any, else LF. Throws
# when there is no Unreleased heading or nothing to release, the version already has a section, or the Unreleased
# link is not a compare link.
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
  $candidates = @()
  if ($Version -notmatch '-rc\.[0-9]+$') { $candidates = Find-ChangelogCandidates $lines }

  $linkBase = $null
  if (-not $candidates.Count) {
    if (-not (Test-ChangelogEntries (Get-ChangelogSection -Text $Text -Name 'Unreleased'))) {
      throw 'CHANGELOG.md has no entries under [Unreleased]: the release would have no notes'
    }
    $lines.Insert($unreleased + 1, '')
    $lines.Insert($unreleased + 2, "## [$Version] - $Date")
  } else {
    $bodies = [Collections.Generic.List[string]]::new()
    for ($i = $candidates.Count - 1; $i -ge 0; $i--) { $bodies.Add((Join-ChangelogBody $lines ($candidates[$i].Start + 1) $candidates[$i].End)) }
    $bodies.Add((Join-ChangelogBody $lines ($unreleased + 1) (Find-ChangelogSectionEnd $lines $unreleased)))
    if (-not @($bodies | Where-Object { Test-ChangelogEntries $_ }).Count) {
      throw "CHANGELOG.md has no entries under [Unreleased] or its release candidates' sections: the release would have no notes"
    }
    $regionEnd = $candidates[$candidates.Count - 1].End
    $merged = [Collections.Generic.List[string]]::new()
    for ($i = 0; $i -le $unreleased; $i++) { $merged.Add($lines[$i]) }
    $merged.AddRange([string[]]@('', "## [$Version] - $Date", ''))
    $merged.AddRange([string[]](Split-ChangelogLines (Merge-ChangelogBodies $bodies.ToArray())))
    if ($regionEnd -lt $lines.Count) {
      $merged.Add('')
      for ($i = $regionEnd; $i -lt $lines.Count; $i++) { $merged.Add($lines[$i]) }
    } elseif ($lines[$lines.Count - 1] -eq '') { $merged.Add('') }
    $oldest = $candidates[$candidates.Count - 1].Name
    $names = @($candidates | ForEach-Object { $_.Name })
    $hidden = Find-ChangelogHiddenLines $merged.ToArray()
    for ($i = $merged.Count - 1; $i -ge 0; $i--) {
      if (-not $hidden[$i] -and $merged[$i] -match '^\[([0-9]+\.[0-9]+\.[0-9]+-rc\.[0-9]+)\]: ' -and $Matches[1] -in $names) {
        if ($Matches[1] -eq $oldest -and $merged[$i] -match '/compare/(\S+?)\.\.\.\S+$') { $linkBase = $Matches[1] }
        $merged.RemoveAt($i)
      }
    }
    $lines = $merged
  }

  $link = -1
  $hidden = Find-ChangelogHiddenLines $lines.ToArray()
  for ($i = 0; $i -lt $lines.Count; $i++) { if (-not $hidden[$i] -and $lines[$i] -match '^\[Unreleased\]: ') { $link = $i; break } }
  if ($link -ge 0) {
    if ($lines[$link] -notmatch '^\[Unreleased\]: (\S+)/compare/(\S+)\.\.\.HEAD$') {
      throw "CHANGELOG.md's [Unreleased] link is not <repo>/compare/<base>...HEAD: $($lines[$link])"
    }
    $repoUrl = $Matches[1]
    $base = if ($candidates.Count) { $linkBase } else { $Matches[2] }
    $lines[$link] = "[Unreleased]: $repoUrl/compare/$Tag...HEAD"
    if ($base) { $lines.Insert($link + 1, "[$Version]: $repoUrl/compare/$base...$Tag") }
  }
  $lines -join $newline
}

Export-ModuleMember -Function Get-ChangelogSection, Get-ChangelogEntryCounts, Test-ChangelogEntries, Get-ChangelogWrappedLines,
  Get-ChangelogCandidates, Get-ChangelogLink, Get-ChangelogCompareBase, Test-ChangelogUnclosed, Update-ChangelogForRelease
