# CHANGELOG.md (Keep a Changelog) for build\Release-D2BS.ps1, which folds the changelog.d entries into the Unreleased
# section and dates it at a release (and at a final release merges the release candidates' sections into it), and
# build\Get-ReleaseNotes.ps1, which hands a version's section to the release build as its GitHub release notes.
Set-StrictMode -Version 3

# A version label: major.minor.patch, or a release candidate's major.minor.patch-rc.N.
$script:finalLabel = '[0-9]+\.[0-9]+\.[0-9]+'
$script:candidateLabel = "$script:finalLabel-rc\.[0-9]+"
$script:versionLabel = "$script:finalLabel(-rc\.[0-9]+)?"
# A "<!--" inside a paragraph hides up to its "-->" when that comes before the paragraph ends: at a blank line, or at
# a line that starts a heading or a list item. One that never closes there is shown as text. "<!-->" and "<!--->" are
# whole comments.
$script:inlineComment = '<!--(?:-?>|(?:(?!\n[ \t]*(?:\n|\z|#{1,6}(?:[ \t]|\n|\z)|(?:[-*+]|[0-9]{1,9}[.)])(?:[ \t]|\n|\z)))[\s\S])*?-->)'
# The order Keep a Changelog gives its groups; a merged section keeps it, with any other group after them.
$script:groupOrder = 'Added', 'Changed', 'Deprecated', 'Removed', 'Fixed', 'Security'

function Split-ChangelogLines([AllowEmptyString()][string]$Text) { , @($Text -split '\r?\n') }

# Which lines carry no Markdown structure on GitHub: an HTML comment block, which it hides, and a fenced code block,
# which it shows as code (its fences included; it closes on a fence of the same character, at least as long, with
# nothing after it). A comment block starts at a line opening with "<!--", or at a list item whose text does, and ends
# at the line holding "-->". Either block inside a list item also ends with the item: a fence just closes there, but
# a comment left open hides all the rest on GitHub, as one left open at the end of the file does. Every reader of
# CHANGELOG.md's structure here skips these lines, so a commented-out template is never taken for a heading, a link or
# an entry. A "<!--" later in a line opens no block: it hides at most its own paragraph's text, where no heading can
# sit.
function Find-ChangelogHiddenLines([string[]]$Lines) {
  $hidden = [bool[]]::new($Lines.Count)
  # An open block's list item, as the column its text starts at (0 outside a list); a comment's is -1 once the item
  # ended with it open.
  $comment = $null
  $fence = $null
  $fenceAt = 0
  # The text columns of the list items the current line is in, innermost last.
  $items = [Collections.Generic.List[int]]::new()
  for ($i = 0; $i -lt $Lines.Count; $i++) {
    $line = $Lines[$i]
    $indent = $line.Length - $line.TrimStart(' ').Length
    $blank = -not $line.Trim()
    # The line that ends a fence's list item is read afresh below.
    if ($fence -and -not $blank -and $indent -lt $fenceAt) { $fence = $null }
    if ($fence) {
      $hidden[$i] = $true
      if ($indent - $fenceAt -le 3 -and $line -match ('^ *' + [regex]::Escape($fence) + [regex]::Escape([string]$fence[0]) + '*[ \t]*$')) { $fence = $null }
    } elseif ($null -ne $comment) {
      $hidden[$i] = $true
      if (-not $blank -and $indent -lt $comment) { $comment = -1 }
      elseif ($comment -ge 0 -and $line.Contains('-->')) { $comment = $null }
    } else {
      # A line indented less than a list item's text ends the item (a lazy paragraph line aside: entries are one line).
      if (-not $blank) { while ($items.Count -and $indent -lt $items[$items.Count - 1]) { $items.RemoveAt($items.Count - 1) } }
      $within = if ($items.Count) { $items[$items.Count - 1] } else { 0 }
      if ($indent - $within -le 3 -and ($line -match '^ *(`{3,})[^`]*$' -or $line -match '^ *(~{3,})')) {
        # A backtick in the info string makes the line inline code, not a fence.
        $hidden[$i] = $true
        $fence = $Matches[1]
        $fenceAt = $within
      } elseif ($line -match '^ *<!--' -and $indent - $within -le 3) {
        # The "--" that opens the comment may start its "-->" too, so "<!-->" closes on its own line.
        $hidden[$i] = $true
        if ($line.IndexOf('-->', $indent + 2) -lt 0) { $comment = $within }
      } elseif ($line -match '^( *(?:[-*+]|[0-9]{1,9}[.)])[ \t]+)(<!--)?') {
        $text = $Matches[1].Length
        $items.Add($text)
        # Closed on its own line, it is left to the inline-comment strip, which keeps any text after it.
        if ($Matches[2] -and $line.IndexOf('-->', $text + 2) -lt 0) { $hidden[$i] = $true; $comment = $text }
      }
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

# The line index of "## [<Name>]" (with or without " - <date>"), or -1; with -Hidden, of one inside an HTML comment or
# fenced code instead. Labels match in any case, as Markdown's do.
function Find-ChangelogHeading([string[]]$Lines, [string]$Name, [switch]$Hidden) {
  $pattern = '^## \[' + [regex]::Escape($Name) + '\]( |$)'
  $mask = Find-ChangelogHiddenLines $Lines
  for ($i = 0; $i -lt $Lines.Count; $i++) { if ($mask[$i] -eq [bool]$Hidden -and $Lines[$i] -match $pattern) { return $i } }
  -1
}

# Whether a section's "## [<Name>]" heading is there only inside an HTML comment or fenced code.
function Test-ChangelogHeadingHidden([AllowEmptyString()][string]$Text, [string]$Name) {
  $lines = Split-ChangelogLines $Text
  (Find-ChangelogHeading $lines $Name) -lt 0 -and (Find-ChangelogHeading $lines $Name -Hidden) -ge 0
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
  while ($at -lt $Lines.Count -and $Lines[$at] -match "^## \[($script:candidateLabel)\]( |$)") {
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

# Whether a name is shaped like a release tag (v1.7.0, v1.7.0-rc.2): lowercase v and rc, as git refs match exactly.
# Whether such a tag exists is for the caller to ask git or GitHub.
function Test-ChangelogTagName([AllowEmptyString()][string]$Name) { $Name -cmatch "^v$script:versionLabel\z" }

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

# A section without the text GitHub hides or shows as code: its HTML comment blocks and fenced code (see
# Find-ChangelogHiddenLines) turn into blank lines, as a paragraph ends at them, and then its inline comments go.
function Remove-ChangelogHidden([AllowEmptyString()][string]$Section) {
  $lines = Split-ChangelogLines $Section
  $hidden = Find-ChangelogHiddenLines $lines
  (@(for ($i = 0; $i -lt $lines.Count; $i++) { if ($hidden[$i]) { '' } else { $lines[$i] } }) -join "`n") -replace $script:inlineComment, ''
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
  if ($Version -notmatch "^$script:candidateLabel\z") { $candidates = Find-ChangelogCandidates $lines }

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
      if (-not $hidden[$i] -and $merged[$i] -match "^\[($script:candidateLabel)\]: " -and $Matches[1] -in $names) {
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

# A changelog.d file name read as <slug>.<section>.md: the slug in lowercase letters and digits, words joined by
# hyphens, and the section one of Keep a Changelog's groups in lowercase. Group is that group's heading (Fixed);
# Problem says why the name is no entry's, and then the release stops rather than leave the file out unseen.
function Read-ChangelogFragmentName([string]$Name) {
  $sections = [string[]]@($script:groupOrder | ForEach-Object { $_.ToLowerInvariant() })
  $slug = $null
  $group = $null
  $problem = $null
  if ($Name -cmatch '^([a-z0-9]+(?:-[a-z0-9]+)*)\.([^.]+)\.md\z') {
    $slug = $Matches[1]
    $section = $Matches[2]
    $at = [Array]::IndexOf($sections, $section)
    if ($at -ge 0) { $group = $script:groupOrder[$at] }
    else { $problem = "unknown section '$section': the name ends in .<section>.md, one of $($sections -join ', ')" }
  } else {
    $problem = 'not named <slug>.<section>.md, the slug in lowercase letters, digits and hyphens'
  }
  [pscustomobject]@{ Slug = $slug; Group = $group; Problem = $problem }
}

# A changelog.d file's text without the line breaks at its end, which an editor may add more than one of.
function Get-ChangelogFragmentLine([AllowEmptyString()][string]$Text) { $Text -replace '(\r?\n[ \t]*)+\z', '' }

# A changelog.d file's bytes as text: strict UTF-8, a BOM dropped. Problem is set instead when they are not UTF-8 or
# hold U+FFFD, which a lenient conversion leaves for each character it could not read.
function ConvertFrom-ChangelogFragmentBytes([byte[]]$Bytes) {
  $skip = if ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF) { 3 } else { 0 }
  $text = $null
  $problem = $null
  try { $text = [Text.UTF8Encoding]::new($false, $true).GetString($Bytes, $skip, $Bytes.Length - $skip) }
  catch { $problem = 'is not valid UTF-8 (save it as UTF-8)' }
  if ($null -ne $text -and $text.Contains([string][char]0xFFFD)) {
    $problem = 'holds U+FFFD, the mark a conversion leaves for a character it could not read: type that character again'
  }
  [pscustomobject]@{ Text = $text; Problem = $problem }
}

# Why a changelog.d file's text can't be one entry, or $null: it is one line (line breaks at its end aside) that
# opens with "- " and no code fence, counts as an entry, and leaves no HTML comment or code fence open to swallow
# the entries after it.
function Get-ChangelogFragmentProblem([AllowEmptyString()][string]$Text) {
  $line = Get-ChangelogFragmentLine $Text
  if (-not $line.Trim()) { return 'is empty: it holds one entry, "- **Lead.** Text."' }
  if ($line.Contains("`n") -or $line.Contains("`r")) { return 'has more than one line: an entry is one line, because the release notes show every line break' }
  if (-not $line.StartsWith('- ', [StringComparison]::Ordinal)) { return 'does not open with "- ": an entry is one list item' }
  # As in Find-ChangelogHiddenLines, a backtick in the info string makes the line inline code, not a fence. GitHub
  # expands a tab after the marker as indentation, so "- <tab>~~~" opens a fence too.
  if ($line -cmatch '^-[ \t]+(~{3,}|`{3,}[^`]*\z)') { return 'opens a code fence after "- ", which shows as an empty list item' }
  if (Test-ChangelogUnclosed $line) { return 'opens an HTML comment or a code fence that never closes' }
  if (-not (Test-ChangelogEntries $line)) { return 'is no entry once its HTML comments are hidden' }
  $null
}

# Where a group sits in Keep a Changelog's order (any case); every other group sits after them all.
function Get-ChangelogGroupRank([string]$Name) {
  for ($i = 0; $i -lt $script:groupOrder.Count; $i++) { if ($script:groupOrder[$i] -eq $Name) { return $i } }
  $script:groupOrder.Count
}

# Inserts lines at $At with a blank line after them unless one follows already; -Separate puts one before them too,
# as a heading needs.
function Add-ChangelogLines($Lines, [int]$At, [string[]]$New, [switch]$Separate) {
  $block = [Collections.Generic.List[string]]::new()
  if ($Separate -and $At -gt 0 -and $Lines[$At - 1].Trim()) { $block.Add('') }
  $block.AddRange($New)
  if ($At -lt $Lines.Count -and $Lines[$At].Trim()) { $block.Add('') }
  $Lines.InsertRange($At, $block)
}

# Adds entries to [Unreleased], each group's in the order given: after the last line under the group's heading (its
# last one, in any case), or under a new heading placed before the first group that Keep a Changelog orders after it
# or doesn't know, as Merge-ChangelogBodies orders them, else at the section's end. Headings inside HTML comments or
# fenced code don't count. $Entries hold Group (a Keep a Changelog group) and Text (one line). Keeps the file's line
# endings.
function Add-ChangelogUnreleased {
  param([Parameter(Mandatory)][AllowEmptyString()][string]$Text, [Parameter(Mandatory)][AllowEmptyCollection()][object[]]$Entries)
  if (-not $Entries.Count) { return $Text }
  $newline = if ($Text.Contains("`r`n")) { "`r`n" } else { "`n" }
  $lines = [Collections.Generic.List[string]]::new([string[]](Split-ChangelogLines $Text))
  $start = Find-ChangelogHeading $lines.ToArray() 'Unreleased'
  if ($start -lt 0) { throw 'CHANGELOG.md has no "## [Unreleased]" heading' }
  foreach ($group in $script:groupOrder) {
    $items = [string[]]@($Entries | Where-Object { $_.Group -eq $group } | ForEach-Object { $_.Text })
    if (-not $items.Count) { continue }
    $all = $lines.ToArray()
    $end = Find-ChangelogSectionEnd $all $start
    $hidden = Find-ChangelogHiddenLines $all
    $heads = @(for ($i = $start + 1; $i -lt $end; $i++) {
        if (-not $hidden[$i] -and $all[$i] -match '^### (.+)$') { [pscustomobject]@{ At = $i; Name = $Matches[1].Trim() } }
      })
    $own = -1
    for ($h = 0; $h -lt $heads.Count; $h++) { if ($heads[$h].Name -eq $group) { $own = $h } }
    if ($own -ge 0) {
      $last = $(if ($own + 1 -lt $heads.Count) { $heads[$own + 1].At } else { $end }) - 1
      while ($last -gt $heads[$own].At -and -not $all[$last].Trim()) { $last-- }
      Add-ChangelogLines $lines ($last + 1) $items -Separate:($last -eq $heads[$own].At)
    } else {
      $rank = Get-ChangelogGroupRank $group
      $next = @($heads | Where-Object { (Get-ChangelogGroupRank $_.Name) -gt $rank })
      if ($next.Count) { $at = $next[0].At }
      else {
        $at = $end
        while ($at - 1 -gt $start -and -not $all[$at - 1].Trim()) { $at-- }
      }
      Add-ChangelogLines $lines $at (@("### $group", '') + $items) -Separate
    }
  }
  $lines -join $newline
}

Export-ModuleMember -Function Get-ChangelogSection, Get-ChangelogEntryCounts, Test-ChangelogEntries, Get-ChangelogWrappedLines,
  Get-ChangelogCandidates, Get-ChangelogLink, Get-ChangelogCompareBase, Test-ChangelogTagName, Test-ChangelogUnclosed,
  Test-ChangelogHeadingHidden, Update-ChangelogForRelease, Read-ChangelogFragmentName, Get-ChangelogFragmentLine,
  ConvertFrom-ChangelogFragmentBytes, Get-ChangelogFragmentProblem, Add-ChangelogUnreleased
