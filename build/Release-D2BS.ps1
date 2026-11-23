#Requires -Version 7.2
<#
  Interactive release for D2BS. Checks that master is ready, proposes the next version: a release candidate (1.7.0-rc.1,
  then 1.7.0-rc.2) or a final (1.7.0), or the same version again when its release commit exists but its tag was
  deleted. It writes VERSION and updates CHANGELOG.md: the entries in changelog.d (one file each) go into the Unreleased
  section, then a release candidate dates that section as its own, and a final merges the release candidates' sections
  and Unreleased into one dated section. After a review (CHANGELOG.md can be edited there) it commits both, with the
  changelog.d files deleted, as "chore(release): v<version>", tags it (annotated) and pushes master and the tag in one
  atomic push. The tag push makes the build workflow publish a release candidate as a GitHub pre-release,
  or create a final as a DRAFT release to publish after a test. Each carries D2BS.dll, D2BS.pdb and D2BS-<tag>.zip
  (the DLL, the PDB and CHANGELOG.md in a d2bs folder, to extract into a kolbot folder), and its notes open with the
  version's CHANGELOG.md section.

    pwsh -File build\Release-D2BS.ps1             interactive
    pwsh -File build\Release-D2BS.ps1 -DryRun     rehearse: prints every step that would change something, changes no
                                                  file and makes no commit or tag; it still fetches origin and its
                                                  tags unless -NoFetch
    pwsh -File build\Release-D2BS.ps1 -NoFetch    skip git fetch and the remote tag lookup
#>
# CmdletBinding makes an unknown argument an error: a mistyped switch (-DryRn, --dry-run) must never run a real
# release.
[CmdletBinding()]
param([switch]$DryRun, [switch]$NoFetch)
Set-StrictMode -Version 3
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$versionFile = Join-Path $repo 'VERSION'
$changelogFile = Join-Path $repo 'CHANGELOG.md'
$fragmentDir = Join-Path $repo 'changelog.d'
Import-Module (Join-Path $PSScriptRoot 'Changelog.psm1') -Force

function Say([string]$s = '') { Write-Host $s }
function Ok([string]$s) { Write-Host '[ok] ' -ForegroundColor Green -NoNewline; Write-Host $s }
function Warn([string]$s) { Write-Host '[!!] ' -ForegroundColor Yellow -NoNewline; Write-Host $s }
function Bad([string]$s) { Write-Host '[xx] ' -ForegroundColor Red -NoNewline; Write-Host $s }
function Stop-Release([string]$Message = 'aborted') { Bad $Message; exit 1 }
# A failed precondition stops a real release; a dry run only warns, so it can be rehearsed from any state.
function Fail([string]$Message) { if ($DryRun) { Warn $Message } else { Stop-Release $Message } }

# Reads one answer from the console or from piped input. Running out of input stops cleanly, or answers -OnEof when
# one is given (after the push, where stopping would report a finished release as failed).
function Ask([string]$Prompt, [string]$OnEof) {
  Write-Host $Prompt -NoNewline
  $line = [Console]::In.ReadLine()
  if ($null -eq $line) {
    Write-Host ''
    if ($PSBoundParameters.ContainsKey('OnEof')) { return $OnEof }
    Stop-Release 'no more input'
  }
  if ([Console]::IsInputRedirected) { Write-Host $line }
  $line.Trim()
}
function Confirm-Step([string]$Prompt, [bool]$DefaultYes, [switch]$EofMeansNo) {
  $question = $Prompt + $(if ($DefaultYes) { ' [Y/n] ' } else { ' [y/N] ' })
  $answer = if ($EofMeansNo) { (Ask $question -OnEof 'n').ToLower() } else { (Ask $question).ToLower() }
  if (-not $answer) { return $DefaultYes }
  $answer -in 'y', 'yes'
}

# git for reading: its output, or $null when it fails.
function Read-Git([string[]]$GitArgs) {
  $out = & git -C $repo @GitArgs 2>$null
  if ($LASTEXITCODE) { return $null }
  (@($out) -join "`n").Trim()
}
# git for a step that changes something: shown, skipped in a dry run; a failure prints how to undo and stops.
function Invoke-GitStep([string[]]$GitArgs, [string[]]$Undo = @()) {
  Say ('  > git ' + (($GitArgs | ForEach-Object { if ($_ -match '\s') { "`"$_`"" } else { $_ } }) -join ' '))
  if ($DryRun) { return }
  & git -C $repo @GitArgs
  if ($LASTEXITCODE) {
    Bad "git $($GitArgs[0]) failed ($LASTEXITCODE)"
    if ($Undo) { Say 'To undo:'; $Undo | ForEach-Object { Say "  $_" } }
    exit 1
  }
}

# major.minor.patch, each at most 65535 (the DLL's file version holds 16-bit numbers), and -rc.N from 1 for a release
# candidate (capped alike, as build\Build-D2BS.ps1 caps it), as a semantic version: 1.7.0-rc.2 sorts after 1.7.0-rc.1
# and before 1.7.0. Any case and leading zeros are read here; ToString() gives the one spelling VERSION and tags use.
function ConvertTo-Version([string]$Text) {
  if ($Text -notmatch '^([0-9]{1,5})\.([0-9]{1,5})\.([0-9]{1,5})(-rc\.([0-9]{1,5}))?$') { return $null }
  $parts = @([int]$Matches[1], [int]$Matches[2], [int]$Matches[3])
  $rc = if ($Matches.Contains(5)) { [int]$Matches[5] } else { 0 }
  if (@($parts | Where-Object { $_ -gt 65535 }).Count -or $rc -gt 65535 -or ($Matches.Contains(5) -and $rc -lt 1)) { return $null }
  if ($rc) { [semver]::new($parts[0], $parts[1], $parts[2], "rc.$rc") } else { [semver]::new($parts[0], $parts[1], $parts[2]) }
}
function Test-Candidate($Version) { [bool]$Version.PreReleaseLabel }
function Test-LocalTag([string]$Tag) { [bool](Read-Git @('tag', '--list', $Tag)) }
# $true or $false when origin answered, $null when it could not be asked (or -NoFetch).
function Test-RemoteTag([string]$Tag) {
  if ($NoFetch) { return $null }
  $remote = Read-Git @('ls-remote', '--tags', 'origin', "refs/tags/$Tag")
  if ($null -eq $remote) { return $null }
  [bool]$remote
}

Say ''
Say ('D2BS release' + $(if ($DryRun) { '  (dry run)' } else { '' }))
Say ('-' * 60)

# === 1. Preconditions ===============================================================================================
Say 'Preconditions'
if ($NoFetch) { Warn '-NoFetch: no git fetch and no remote tag lookup; master is compared with the last fetched origin' }
elseif ($null -eq (Read-Git @('fetch', 'origin', '--tags'))) { Warn 'could not fetch origin; the checks below may be stale' }
else { Ok 'fetched origin and its tags' }

$branch = Read-Git @('rev-parse', '--abbrev-ref', 'HEAD')
if ($branch -eq 'master') { Ok 'on branch master' } else { Fail "on branch '$branch', not master: releases ship only from master" }
$headAtStart = Read-Git @('rev-parse', 'HEAD')

$status = @((Read-Git @('status', '--porcelain', '--untracked-files=all')) -split "`n" | Where-Object { $_ })
$tracked = @($status | Where-Object { -not $_.StartsWith('??') })
# changelog.d's own files are named by its checks below, which stop a release on an uncommitted one.
$untracked = @($status | Where-Object { $_.StartsWith('??') } | ForEach-Object { $_.Substring(3) } | Where-Object { -not $_.StartsWith('changelog.d/') })
if ($tracked.Count) { Fail "the working tree has $($tracked.Count) tracked change(s); the release commit must hold only VERSION, CHANGELOG.md and the changelog.d files it folds in" }
else { Ok 'no tracked changes' }
if ($untracked.Count) { Warn "untracked files (left out of the release): $(($untracked | Select-Object -First 5) -join ', ')$(if ($untracked.Count -gt 5) { ', ...' })" }

$behind = Read-Git @('rev-list', '--count', 'master..origin/master')
$ahead = Read-Git @('rev-list', '--count', 'origin/master..master')
if ($null -eq $behind -or $null -eq $ahead) { Fail 'could not compare master with origin/master' }
elseif ([int]$behind -gt 0) { Fail "master is $behind commit(s) behind origin/master; pull first" }
elseif ([int]$ahead -gt 0) {
  Warn "master is $ahead commit(s) ahead of origin/master; the release push publishes them too:"
  (Read-Git @('log', '--oneline', 'origin/master..master')) -split "`n" | ForEach-Object { Say "      $_" }
  if (-not $DryRun -and -not (Confirm-Step 'Continue and publish these commits?' $false)) { Stop-Release }
} else { Ok 'master is level with origin/master' }

# Get-Content -Raw outputs nothing for an empty file, which even a [string] cast leaves $null (unlike [string]$null),
# so check for it. (No ??: Windows PowerShell must parse this file to show #Requires' own message.)
function Read-TextFile([string]$Path) { $text = Get-Content -LiteralPath $Path -Raw -Encoding utf8; if ($null -eq $text) { '' } else { $text } }
$current = if (Test-Path -LiteralPath $versionFile) { (Read-TextFile $versionFile).Trim() } else { '' }
$currentVersion = ConvertTo-Version $current
if (-not $currentVersion -or $currentVersion.ToString() -cne $current) {
  Stop-Release "VERSION must hold major.minor.patch or major.minor.patch-rc.N without leading zeros, not '$current'"
}

function Read-Changelog { Read-TextFile $changelogFile }
if (-not (Test-Path -LiteralPath $changelogFile -PathType Leaf)) { Stop-Release 'CHANGELOG.md is missing: the release notes come from it' }
# The review and its undo work through git, which can't put back a file it doesn't track.
if (-not (Read-Git @('ls-files', '--', 'CHANGELOG.md'))) { Fail 'CHANGELOG.md is not committed: commit it to master first' }
$changelog = Read-Changelog
# First, since a block that never closes hides the headings, links and entries the checks after it look for.
if (Test-ChangelogUnclosed $changelog) {
  Warn 'CHANGELOG.md has an HTML comment or a code fence that never closes: GitHub hides the rest of the file or shows it as code, and this script reads no headings, links or entries there'
}
$unreleased = Get-ChangelogSection -Text $changelog -Name 'Unreleased'
if ($null -eq $unreleased) { Stop-Release 'CHANGELOG.md has no "## [Unreleased]" heading' }
if ($null -eq (Get-ChangelogLink -Text $changelog -Name 'Unreleased')) { Warn 'CHANGELOG.md has no [Unreleased] link at its end, so the release adds no compare link' }

# The changelog.d entries, in the order they go into [Unreleased]: each group's in the order their files reached
# master, by where the commit that added each one sits on its first-parent history (a merge commit brings in its
# branch's files), files added together by name, and files not committed last, by name. Every other file but its
# README is a problem, as is a slug used twice: a real release stops on them rather than leave an entry out unseen.
# An uncommitted file is one too, since the release commit deletes the files it folds in, and so is a tracked file
# missing from the working tree, whose entry would stay out while its file stays committed.
function Read-Fragments {
  $entries = [Collections.Generic.List[object]]::new()
  $problems = [Collections.Generic.List[string]]::new()
  # Before the folder check: a sparse checkout can leave out the whole folder, and git status reports its files clean.
  # Unquoted paths, so they compare with the folder listing. Valid names are ASCII: only a name already refused can
  # still differ, as the console code page decodes git's output.
  $listed = Read-Git @('-c', 'core.quotePath=false', 'ls-files', '--', 'changelog.d')
  if ($null -eq $listed) { $problems.Add("git could not list changelog.d's tracked files") }
  $tracked = @("$listed" -split "`n" | Where-Object { $_ })
  foreach ($path in $tracked) {
    if ($path -cne 'changelog.d/README.md' -and -not (Test-Path -LiteralPath (Join-Path $repo $path) -PathType Leaf)) {
      $problems.Add("${path}: tracked by git but missing here (deleted, or outside a sparse checkout): restore it, or commit its deletion")
    }
  }
  # A committed file changed after the clean-tree check (while the publish prompt waited) would go in as changed, and
  # git rm refuses it at the commit step. Untracked or newly added files and worktree deletions have their own
  # problems ("not committed", "missing here").
  $statusLines = Read-FragmentStatus
  if ($null -eq $statusLines) { $problems.Add("git could not read changelog.d's status") }
  foreach ($line in @($statusLines | Where-Object { $_ })) {
    if ($line.StartsWith('??') -or $line[0] -ceq 'A' -or $line.StartsWith(' D') -or $line.Substring(3) -ceq 'changelog.d/README.md') { continue }
    $problems.Add("$($line.Substring(3)): changed since its commit (git status '$($line.Substring(0, 2))'): commit the change, or restore the file")
  }
  if (-not (Test-Path -LiteralPath $fragmentDir -PathType Container)) { return [pscustomobject]@{ Entries = @(); Problems = $problems.ToArray() } }
  # --root lists a shallow clone's boundary commit as adding its files, whatever log.showRoot says.
  $history = Read-Git @('-c', 'core.quotePath=false', 'log', '--root', '--first-parent', '--diff-merges=first-parent', '--reverse', '--no-renames', '--diff-filter=A',
    '--name-only', '--format=tformat:@', 'HEAD', '--', 'changelog.d')
  if ($null -eq $history) { $problems.Add("git could not read changelog.d's history, which orders its entries") }
  # The last time each path was added: a slug reused after a release deleted its file sits where it came back.
  $addedAt = @{}
  $position = 0
  foreach ($line in @("$history" -split "`n")) { if ($line -ceq '@') { $position++ } elseif ($line) { $addedAt[$line] = $position } }
  $found = @(Get-ChildItem -LiteralPath $fragmentDir -Force | Where-Object { -not ($_.Name -ceq 'README.md' -and -not $_.PSIsContainer) } |
      ForEach-Object { [pscustomobject]@{ Item = $_; Name = $(if (-not $_.PSIsContainer) { Read-ChangelogFragmentName $_.Name }) } })
  # Slugs are compared across every file whose name reads, before their text is checked, so a problem in one file's
  # text can't let the other file of a pair in.
  $shared = @{}
  foreach ($same in @($found | Where-Object { $_.Name -and $_.Name.Slug } | Group-Object { $_.Name.Slug } | Where-Object { $_.Count -gt 1 })) {
    $problems.Add("$(@($same.Group | ForEach-Object { "changelog.d/$($_.Item.Name)" }) -join ', '): the same slug twice, so neither goes in")
    $shared[$same.Name] = $true
  }
  foreach ($f in $found) {
    $item = $f.Item
    $name = $f.Name
    $path = "changelog.d/$($item.Name)"
    if ($item.PSIsContainer) { $problems.Add("${path}: a folder, which holds no entry"); continue }
    if ($name.Problem) { $problems.Add("${path}: $($name.Problem)"); continue }
    try { $bytes = [IO.File]::ReadAllBytes($item.FullName) }
    catch { $problems.Add("${path}: could not be read ($($_.Exception.Message))"); continue }
    $decoded = ConvertFrom-ChangelogFragmentBytes $bytes
    $problem = if ($decoded.Problem) { $decoded.Problem } else { Get-ChangelogFragmentProblem $decoded.Text }
    if ($problem) { $problems.Add("${path}: $problem"); continue }
    $committed = $path -cin $tracked -and $addedAt.ContainsKey($path)
    if (-not $committed) { $problems.Add("${path}: not committed, and the release commit deletes it: commit it first") }
    if ($shared.ContainsKey($name.Slug)) { continue }
    $entries.Add([pscustomobject]@{
        Path = $path; File = $item.FullName; Name = $item.Name; Slug = $name.Slug; Group = $name.Group
        Text = (Get-ChangelogFragmentLine $decoded.Text); Bytes = $bytes
        Position = $(if ($committed) { $addedAt[$path] } else { [int]::MaxValue })
      })
  }
  $entries.Sort([Comparison[object]] { param($a, $b)
      $byPosition = $a.Position.CompareTo($b.Position)
      if ($byPosition) { $byPosition } else { [string]::CompareOrdinal($a.Name, $b.Name) }
    })
  [pscustomobject]@{ Entries = $entries.ToArray(); Problems = $problems.ToArray() }
}
# Whether a file still holds exactly these bytes. One that can't be read (locked, access denied) counts as changed
# rather than throwing, so the cleanup after the review still puts VERSION and CHANGELOG.md back.
function Test-SameBytes([string]$Path, [byte[]]$Bytes) {
  try { (Test-Path -LiteralPath $Path -PathType Leaf) -and [Linq.Enumerable]::SequenceEqual([byte[]][IO.File]::ReadAllBytes($Path), $Bytes) }
  catch { $false }
}
# The paths of everything in changelog.d, git's view aside, so an ignored file shows too.
function Get-FragmentNames {
  if (Test-Path -LiteralPath $fragmentDir -PathType Container) { @(Get-ChildItem -LiteralPath $fragmentDir -Force | ForEach-Object { "changelog.d/$($_.Name)" }) }
}
# changelog.d's git status lines as git prints them (Read-Git's trim would cut the first one's status column), or $null
# when git fails.
function Read-FragmentStatus {
  $out = & git -C $repo status --porcelain --untracked-files=all -- changelog.d 2>$null
  if ($LASTEXITCODE) { return $null }
  , [string[]]@($out | Where-Object { $_ })
}
# The changelog.d paths that differ from what the plan read: a file added, removed, renamed or edited, or another
# change in git's status. -AllowDeleted passes a folded file gone missing, which the release commit deletes anyway.
function Get-FragmentChanges([switch]$AllowDeleted) {
  $gone = @(if ($AllowDeleted) { $folded | Where-Object { -not (Test-Path -LiteralPath $_.File) } | ForEach-Object { $_.Path } })
  $changes = [Collections.Generic.List[string]]::new()
  function Add-Change([string]$Path) { if ($Path -cnotin $changes) { $changes.Add($Path) } }
  $statusNow = Read-FragmentStatus
  if ($null -eq $statusNow -or $null -eq $fragmentStatus) { Add-Change 'changelog.d (git status failed)' }
  $statusNow = @($statusNow | Where-Object { $_ -and -not ($_.StartsWith(' D ') -and $_.Substring(3) -cin $gone) })
  $statusThen = @($fragmentStatus | Where-Object { $_ })
  foreach ($line in @($statusNow | Where-Object { $_ -cnotin $statusThen }) + @($statusThen | Where-Object { $_ -cnotin $statusNow })) {
    Add-Change $line.Substring(3)
  }
  $namesNow = @(Get-FragmentNames)
  foreach ($path in @($namesNow | Where-Object { $_ -cnotin $fragmentNames }) + @($fragmentNames | Where-Object { $_ -cnotin $namesNow -and $_ -cnotin $gone })) {
    Add-Change $path
  }
  foreach ($e in $folded) { if ($e.Path -cnotin $gone -and -not (Test-SameBytes $e.File $e.Bytes)) { Add-Change $e.Path } }
  $changes.ToArray()
}
$fragments = Read-Fragments
$fragmentStatus = Read-FragmentStatus
$fragmentNames = @(Get-FragmentNames)
# The text every later step reads; CHANGELOG.md on disk changes only at the review.
$assembled = Add-ChangelogUnreleased -Text $changelog -Entries $fragments.Entries
$unreleased = Get-ChangelogSection -Text $assembled -Name 'Unreleased'

# GitHub extras (the CI check and watching the release build) need gh and a GitHub origin; without them they are skipped.
$slug = $null
if ((Read-Git @('remote', 'get-url', 'origin')) -match 'github\.com[:/]([^/]+)/([^/]+?)(\.git)?$') { $slug = "$($Matches[1])/$($Matches[2])" }
$hasGh = [bool](Get-Command gh -ErrorAction SilentlyContinue)
if (-not $slug) { Warn 'origin is not on GitHub: no CI check and no release watch' }
elseif (-not $hasGh) { Warn 'gh is not installed: no CI check and no release watch' }
else {
  $head = Read-Git @('rev-parse', 'HEAD')
  $runs = & gh run list --repo $slug --workflow build.yml --branch master --limit 5 --json headSha,status,conclusion,url 2>$null
  if ($LASTEXITCODE) { Warn "gh could not list $slug's runs (check gh auth status): no CI check" }
  else {
    $runForHead = if ($runs) { @($runs | ConvertFrom-Json | Where-Object { $_.headSha -eq $head }) | Select-Object -First 1 } else { $null }
    if (-not $runForHead) { Warn "no CI build of master's latest commit found; the release build will be its first" }
    elseif ($runForHead.conclusion -eq 'success') { Ok "CI built master's latest commit: $($runForHead.url)" }
    else {
      Warn "CI on master's latest commit is $($runForHead.status) $($runForHead.conclusion): $($runForHead.url)"
      if (-not $DryRun -and -not (Confirm-Step 'Release anyway?' $false)) { Stop-Release }
    }
  }
}

# === 2. Context =====================================================================================================
Say ''
Say 'Context'
$releaseTags = @((Read-Git @('tag', '--list', 'v*')) -split "`n" | Where-Object { $_ -and (ConvertTo-Version $_.Substring(1)) })
$latestTag = $releaseTags | Sort-Object { ConvertTo-Version $_.Substring(1) } | Select-Object -Last 1
Say "  VERSION: $current   (latest release tag: $(if ($latestTag) { $latestTag } else { 'none' }))"
$range = if ($latestTag) { "$latestTag..HEAD" } else { 'HEAD' }
$log = @((Read-Git @('log', '--oneline', '--no-merges', $range)) -split "`n" | Where-Object { $_ })
if ($latestTag) {
  Say "  $($log.Count) commit(s) since ${latestTag}:"
  $log | Select-Object -First 15 | ForEach-Object { Say "      $_" }
  if ($log.Count -gt 15) { Say "      ... and $($log.Count - 15) more" }
  if ($log.Count -eq 0) { Warn "nothing has changed since $latestTag" }
} else { Say '  This is the first release tag.' }
# The entries per group, e.g. "Added 2, Fixed 5": Semantic Versioning makes an Added entry a minor version.
function Format-EntryCounts($Counts) { ($Counts.Keys | ForEach-Object { "$_ $($Counts[$_])" }) -join ', ' }
$groups = Get-ChangelogEntryCounts $unreleased
# The release candidates' sections since the last final release, which the next final merges.
$candidates = Get-ChangelogCandidates $changelog
foreach ($name in $candidates) {
  $counts = Get-ChangelogEntryCounts (Get-ChangelogSection -Text $changelog -Name $name)
  Say "  CHANGELOG.md [$name]: $(if ($counts.Count) { Format-EntryCounts $counts } else { 'no entries' })"
}
if ($candidates.Count) { Say '  A final release merges these release candidates and [Unreleased] into one section.' }
if ($fragments.Entries.Count) {
  Say "  changelog.d: $($fragments.Entries.Count) entr$(if ($fragments.Entries.Count -eq 1) { 'y' } else { 'ies' }) for [Unreleased], in this order:"
  foreach ($e in $fragments.Entries) { Say "      $($e.Group): $($e.Path)" }
}
foreach ($p in $fragments.Problems) { Warn "changelog.d: $p" }
if ($DryRun -and $fragments.Entries.Count) {
  Say '  [Unreleased] with them:'
  $unreleased -split "`n" | ForEach-Object { Say "    $_" }
}
if ($groups.Count) {
  Say "  CHANGELOG.md [Unreleased]: $(Format-EntryCounts $groups)$(if ($fragments.Entries.Count) { ', changelog.d included' })"
  if ($groups.Contains('Added')) { Say '  It has Added entries: Semantic Versioning makes that a minor version.' }
} elseif ($candidates.Count) {
  Say '  CHANGELOG.md has no entries under [Unreleased]: a final can still merge the release candidates, but another candidate needs new entries there first'
} else {
  Warn 'CHANGELOG.md has no entries under [Unreleased]: a new version needs them there first, or in changelog.d'
}
# The next version's compare link starts where [Unreleased]'s does (a final's where its oldest release candidate's
# does), and the tag build starts the notes' list of pull requests there, so that tag must still exist.
$linkStarts = [ordered]@{ 'Unreleased' = Get-ChangelogCompareBase -Text $changelog -Name 'Unreleased' }
if ($candidates.Count) { $linkStarts[$candidates[$candidates.Count - 1]] = Get-ChangelogCompareBase -Text $changelog -Name $candidates[$candidates.Count - 1] }
foreach ($name in $linkStarts.Keys) {
  $base = $linkStarts[$name]
  if ((Test-ChangelogTagName $base) -and -not (Test-LocalTag $base)) {
    # [Unreleased]'s link starts at VERSION's own tag, which "again" can recreate; an older one can't come back.
    $consequence = if ($name -eq 'Unreleased') { "if its build failed, release $($base.Substring(1)) again first; otherwise the next release's" } else { "a final's" }
    Warn "CHANGELOG.md's [$name] link starts at $base, which is no local tag: $consequence list of pull requests starts wherever GitHub picks"
  }
}

# === 3. Version =====================================================================================================
Say ''
Say 'Version'
$options = @()
# The same version again only when its release commit exists but its tag is gone (deleted after a failed build).
$currentTag = "v$current"
$releaseCommit = Read-Git @('log', '-1', '--format=%h', '--extended-regexp', "--grep=^chore\(release\): v$($current -replace '\.', '\.')$")
$sameAgain = [bool]($releaseCommit -and -not (Test-LocalTag $currentTag) -and -not (Test-RemoteTag $currentTag))
if ($sameAgain) {
  $options += [pscustomobject]@{ Label = "$current again (release commit $releaseCommit, tag deleted)"; Version = $current }
}
if (Test-Candidate $currentVersion) {
  # Inside a release candidate cycle: the next candidate, or the final of the version they lead to.
  $final = '{0}.{1}.{2}' -f $currentVersion.Major, $currentVersion.Minor, $currentVersion.Patch
  $rc = [int]($currentVersion.PreReleaseLabel -replace '^rc\.', '')
  $options += [pscustomobject]@{ Label = 'next release candidate'; Version = "$final-rc.$($rc + 1)" }
  $options += [pscustomobject]@{ Label = 'final'; Version = $final }
} else {
  $next = [ordered]@{
    patch = '{0}.{1}.{2}' -f $currentVersion.Major, $currentVersion.Minor, ($currentVersion.Patch + 1)
    minor = '{0}.{1}.0' -f $currentVersion.Major, ($currentVersion.Minor + 1)
    major = '{0}.0.0' -f ($currentVersion.Major + 1)
  }
  foreach ($kind in $next.Keys) { $options += [pscustomobject]@{ Label = "$kind release candidate"; Version = "$($next[$kind])-rc.1" } }
  foreach ($kind in $next.Keys) { $options += [pscustomobject]@{ Label = $kind; Version = $next[$kind] } }
}
for ($i = 0; $i -lt $options.Count; $i++) { Say ("  {0}) {1} -> v{2}" -f ($i + 1), $options[$i].Label, $options[$i].Version) }
# Inside a cycle with no new entries, another release candidate would have no notes, so the final is the default.
$default = if ((Test-Candidate $currentVersion) -and -not $groups.Count -and -not $sameAgain) { $options.Count } else { 1 }
$pick = $null
while (-not $pick) {
  $answer = Ask "Choice [$default]: "
  if (-not $answer) { $answer = "$default" }
  if ($answer -match '^\d+$' -and [int]$answer -ge 1 -and [int]$answer -le $options.Count) { $pick = $options[[int]$answer - 1] }
  else { Warn "enter 1-$($options.Count)" }
}
$chosen = $null
while (-not $chosen) {
  $candidate = Ask "Version [$($pick.Version)]: "
  if (-not $candidate) { $candidate = $pick.Version }
  $candidate = $candidate.TrimStart('v')
  $candidateVersion = ConvertTo-Version $candidate
  if (-not $candidateVersion) { Warn 'a version is major.minor.patch, or major.minor.patch-rc.N for a release candidate: e.g. 1.6.5 or 1.6.5-rc.1'; continue }
  # Tags and VERSION use the normalized form, so a leading-zero spelling can't duplicate an existing release.
  $candidate = $candidateVersion.ToString()
  if (Test-LocalTag "v$candidate") { Warn "tag v$candidate already exists here"; continue }
  $remote = Test-RemoteTag "v$candidate"
  if ($remote) { Warn "tag v$candidate already exists on origin"; continue }
  if ($null -eq $remote -and -not $NoFetch) { Warn "could not ask origin about v$candidate; the atomic push still refuses a duplicate" }
  # Newer than the latest release tag and than VERSION (the last version cut, which may have no tag: 1.6.4U shipped
  # without one at the time); only the "again" case may reuse VERSION's number.
  $notNewer = if ($latestTag -and $candidateVersion -le (ConvertTo-Version $latestTag.Substring(1))) { $latestTag }
  elseif ($candidateVersion -lt $currentVersion -or ($candidateVersion -eq $currentVersion -and -not $sameAgain)) { "VERSION $current" }
  if ($notNewer -and -not (Confirm-Step "v$candidate is not newer than $notNewer. Use it anyway?" $false)) { continue }
  $chosen = $candidate
}
$tag = "v$chosen"
$bump = $chosen -ne $current
$isCandidate = Test-Candidate (ConvertTo-Version $chosen)
# A final merges the release candidates' sections into its own; a release candidate dates [Unreleased] alone.
$merging = @(if (-not $isCandidate) { $candidates })
# The tag build posts the version's CHANGELOG.md section as the release notes and fails without one.
$date = Get-Date -Format 'yyyy-MM-dd'
# A block left open inside the version's section takes the rest of the file into its notes.
$unclosedNotes = "an HTML comment or a code fence in the [$chosen] notes never closes, so they would run to the end of CHANGELOG.md"
$newChangelog = $null
# A re-release commits nothing, so it leaves changelog.d alone.
$folded = @(if ($bump) { $fragments.Entries })
if ($bump) {
  if ($fragments.Problems.Count) {
    Fail "changelog.d has $($fragments.Problems.Count) problem(s), listed above: fix them, or move those files out of changelog.d"
  }
  try { $newChangelog = Update-ChangelogForRelease -Text $assembled -Version $chosen -Date $date -Tag $tag }
  catch { Fail $_.Exception.Message }
  $notes = if ($newChangelog) { Get-ChangelogSection -Text $newChangelog -Name $chosen } else { $unreleased }
  if (Test-ChangelogUnclosed $notes) { Fail $unclosedNotes }
} else {
  $notes = Get-ChangelogSection -Text $changelog -Name $chosen
  if ($null -eq $notes) {
    $cause = if (Test-ChangelogHeadingHidden $changelog $chosen) { ' (it is inside an HTML comment or a code fence)' } else { '' }
    Fail "CHANGELOG.md has no [$chosen] heading${cause}: the release would have no notes"
  } elseif (Test-ChangelogUnclosed $notes) { Fail $unclosedNotes }
  elseif (-not (Test-ChangelogEntries $notes)) { Fail "CHANGELOG.md has no entries under [$chosen]: the release would have no notes" }
  # A fix made for a re-release belongs to the version it fixes, which is the section the release posts.
  $waiting = @(if (Test-ChangelogEntries (Get-ChangelogSection -Text $changelog -Name 'Unreleased')) { '[Unreleased]' }) +
    @(if ($fragments.Entries.Count) { 'changelog.d' })
  if ($waiting.Count) {
    Warn "$($waiting -join ' and ') $(if ($waiting.Count -gt 1) { 'have' } else { 'has' }) entries, and re-releasing $tag posts only [$chosen]: move a fix's entry there first"
  }
}
# A final is meant to be the code its last release candidate's testers ran; its own release commit (there when it is
# released again) changes only VERSION and CHANGELOG.md.
$untested = @($log | Where-Object { $_ -notmatch '^\S+ chore\(release\): ' })
if (-not $isCandidate -and $latestTag -and (Test-Candidate (ConvertTo-Version $latestTag.Substring(1))) -and $untested.Count) {
  Warn "$tag would ship $($untested.Count) commit(s) that no release candidate had (since $latestTag): cut another release candidate to have them tested"
}

# === 4. Plan ========================================================================================================
Say ''
Say ('-' * 60)
Say 'Release plan'
Say "  Version: $chosen  (tag $tag)"
Say '  Steps:'
$mergedNames = "[$($merging -join '], [')]"
$step = 0
function Step([string]$s) { $script:step++; Say "    $script:step. $s" }
$foldedFiles = "the $($folded.Count) changelog.d file$(if ($folded.Count -ne 1) { 's' })"
if ($bump) {
  Step "write VERSION: $current -> $chosen"
  if ($folded.Count) { Step "add the entries of $foldedFiles to CHANGELOG.md's [Unreleased] section" }
  if ($merging.Count) { Step "merge CHANGELOG.md's $mergedNames and [Unreleased] sections into [$chosen] - $date" }
  else { Step "date CHANGELOG.md's Unreleased section: [$chosen] - $date" }
  Step 'review the change; CHANGELOG.md can be edited there'
  if ($folded.Count) { Step "commit VERSION and CHANGELOG.md, deleting $foldedFiles, as `"chore(release): $tag`"" }
  else { Step "commit VERSION and CHANGELOG.md as `"chore(release): $tag`"" }
} else {
  Step "VERSION already says ${chosen}: the tag goes on HEAD ($(Read-Git @('log', '-1', '--format=%h %s')))"
}
Step "annotated tag $tag"
$built = if ($isCandidate) { 'builds and publishes a pre-release' } else { 'builds a DRAFT release' }
Step "push master and $tag in one atomic push; the tag $built"
if ($notes) {
  $withFolded = if ($folded.Count) { " with $foldedFiles" } else { '' }
  $source = if (-not $bump) { "[$chosen]" } elseif ($merging.Count) { "[Unreleased]$withFolded and $mergedNames, merged" } else { "[Unreleased]$withFolded" }
  Say "  Release notes (CHANGELOG.md ${source}):"
  $notes -split "`n" | ForEach-Object { Say "    $_" }
  foreach ($line in Get-ChangelogWrappedLines $notes) { Warn "a wrapped line shows as its own line on GitHub; join it to its entry: $($line.Trim())" }
}
Say ('-' * 60)
if (-not (Confirm-Step 'Proceed?' $true)) { Stop-Release }
# The new CHANGELOG.md was made from the file as read at the start, and the plan named the commit to tag: an edit or
# a commit made during the prompts would be lost or published unseen.
if (-not $DryRun -and ((Read-Changelog) -cne $changelog -or (Read-Git @('status', '--porcelain', '--', 'VERSION', 'CHANGELOG.md')) -or
    (Read-Git @('rev-parse', 'HEAD')) -ne $headAtStart -or ($bump -and @(Get-FragmentChanges).Count))) {
  Stop-Release 'VERSION, CHANGELOG.md, changelog.d or HEAD changed during the prompts: run this again'
}

# === 5. Prepare and review ==========================================================================================
if ($bump) {
  Say ''
  Say 'Preparing'
  Say "  > write VERSION: $chosen"
  $intoUnreleased = if ($folded.Count) { "[Unreleased] with $foldedFiles" } else { '[Unreleased]' }
  if ($merging.Count) { Say "  > write CHANGELOG.md: $mergedNames and $intoUnreleased -> [$chosen] - $date" }
  else { Say "  > write CHANGELOG.md: $intoUnreleased -> [$chosen] - $date" }
  Say ''
  Say 'Review'
  if ($DryRun) { Say '  (dry run: nothing written, no review)' }
  else {
    # Leaving the review any way but y (n, the end of piped input, Ctrl+C) puts VERSION, CHANGELOG.md and the folded
    # changelog.d files back. Those stay on disk until the commit step removes them, so only a folded file edited or
    # deleted here (a renamed one's old name included) is written back; a file added here, or a new name, stays.
    $approved = $false
    $written = $false
    try {
      [IO.File]::WriteAllText($versionFile, "$chosen`n")
      [IO.File]::WriteAllText($changelogFile, $newChangelog)
      $written = $true
      & git -C $repo --no-pager diff --stat
      if ($folded.Count) { Say "  The release commit deletes $foldedFiles, whose entries are in CHANGELOG.md now:"; $folded | ForEach-Object { Say "      $($_.Path)" } }
      Say '  CHANGELOG.md can be edited before you answer; (d) shows the diff as it is then.'
      if ($merging.Count) {
        Say "  [$chosen] holds every release candidate's entries: drop the ones for problems that only a release candidate had."
      }
      while ($true) {
        $answer = (Ask '(y) commit + tag + push  (d) show the diff  (n) abort: ').ToLower()
        if ($answer -in 'y', 'yes') {
          # A commit made while this prompt waited would go out on top of the release unseen.
          if ((Read-Git @('rev-parse', 'HEAD')) -ne $headAtStart) { Stop-Release 'HEAD changed during the review: run this again' }
          # The commit takes the files as they are now, edits included; the tag build needs the version's notes.
          $versionNow = if (Test-Path -LiteralPath $versionFile) { (Read-TextFile $versionFile).Trim() } else { '' }
          if ($versionNow -cne $chosen) { Warn "VERSION no longer says ${chosen}: put it back, or answer n"; continue }
          $changelogNow = if (Test-Path -LiteralPath $changelogFile) { Read-Changelog } else { '' }
          $notesNow = Get-ChangelogSection -Text $changelogNow -Name $chosen
          if ($null -eq $notesNow) {
            $cause = if (Test-ChangelogHeadingHidden $changelogNow $chosen) { ' (it is inside an HTML comment or a code fence)' } else { '' }
            Warn "CHANGELOG.md has no [$chosen] heading now${cause}: put it back, or answer n"; continue
          }
          if (Test-ChangelogUnclosed $notesNow) { Warn "$unclosedNotes (close it, or answer n)"; continue }
          if (-not (Test-ChangelogEntries $notesNow)) { Warn "CHANGELOG.md has no entries under [$chosen] now: the release would have no notes"; continue }
          # The commit deletes the files it folded in, so an edit made to one now would go nowhere, and a file added
          # or renamed now would stay behind with its entry already in CHANGELOG.md.
          $changed = @(Get-FragmentChanges -AllowDeleted)
          $edited = @($folded | Where-Object { $_.Path -cin $changed } | ForEach-Object { $_.Path })
          if ($edited.Count) { Warn "$($edited -join ', ') changed during the review, and the commit deletes it: make the edit in CHANGELOG.md, or answer n"; continue }
          if ($changed.Count) {
            Warn "changelog.d changed during the review ($($changed -join ', ')), and the commit deletes only the files the plan named: undo that, or answer n"
            continue
          }
          $approved = $true; break
        }
        if ($answer -eq 'd') { & git -C $repo --no-pager diff -- VERSION CHANGELOG.md; continue }
        if ($answer -in 'n', 'no') { Stop-Release }
        Warn 'enter y, d or n'
      }
    } finally {
      if (-not $approved) {
        # The review invites edits to CHANGELOG.md, and the restore below would drop them: keep a copy outside the repo.
        # Only once the release's own text was written: before that, the file on disk is the one the release found.
        try {
          if ($written -and (Test-Path -LiteralPath $changelogFile) -and (Read-Changelog) -cne $newChangelog) {
            $kept = Join-Path ([IO.Path]::GetTempPath()) "d2bs-CHANGELOG-$(Get-Date -Format 'yyyyMMdd-HHmmss')-$PID.md"
            Copy-Item -LiteralPath $changelogFile -Destination $kept
            Write-Host "Your edits to CHANGELOG.md are kept in $kept"
          }
        } catch { Write-Host "Could not keep a copy of your edits to CHANGELOG.md: $($_.Exception.Message)" -ForegroundColor Red }
        # A changelog.d file deleted or edited during the review gets its own bytes back, not git's checkout of it,
        # whose line endings (core.autocrlf) may differ; an edited one is kept outside the repo first.
        foreach ($e in $folded) {
          if (Test-SameBytes $e.File $e.Bytes) { continue }
          try {
            if (Test-Path -LiteralPath $e.File) {
              $kept = Join-Path ([IO.Path]::GetTempPath()) "d2bs-$($e.Name)-$(Get-Date -Format 'yyyyMMdd-HHmmss')-$PID.md"
              Copy-Item -LiteralPath $e.File -Destination $kept
              Write-Host "Your edits to $($e.Path) are kept in $kept"
            }
            [IO.File]::WriteAllBytes($e.File, $e.Bytes)
          } catch { Write-Host "Could not put $($e.Path) back: $($_.Exception.Message)" -ForegroundColor Red }
        }
        # git restore copies from the index, so a commit made during the review that took the release files keeps
        # them: compare with the commit the release started from (worktree and index) before saying they are back.
        & git -C $repo restore -- VERSION CHANGELOG.md
        $restored = -not $LASTEXITCODE
        $paths = @('VERSION', 'CHANGELOG.md') + @($folded | ForEach-Object { $_.Path })
        & git -C $repo diff --quiet $headAtStart -- @paths
        $asFound = $restored -and -not $LASTEXITCODE
        & git -C $repo diff --quiet --cached $headAtStart -- @paths
        if ($asFound -and -not $LASTEXITCODE) {
          Write-Host "VERSION put back to $current, and CHANGELOG.md with it$(if ($folded.Count) { ", and $foldedFiles as they were" })"
        } else {
          Write-Host ('VERSION, CHANGELOG.md and the changelog.d files are not as the release found them (a commit made ' +
            'during the review may hold them): check git log and git status') -ForegroundColor Red
        }
      }
    }
  }
}

# === 6. Commit, tag, push ===========================================================================================
Say ''
Say 'Committing, tagging, pushing'
if ($bump) {
  $releasePaths = @('VERSION', 'CHANGELOG.md') + @($folded | ForEach-Object { $_.Path })
  $undoFiles = @("git restore --staged --worktree -- $($releasePaths -join ' ')")
  # The steps below are separate git calls, and a Ctrl+C between them prints nothing, so the undo comes first.
  if (-not $DryRun) { Say "  Should these steps stop before the commit, this undoes them: $($undoFiles[0])" }
  Invoke-GitStep @('add', '--', 'VERSION', 'CHANGELOG.md') $undoFiles
  if ($folded.Count) { Invoke-GitStep (@('rm', '--quiet', '--') + @($folded | ForEach-Object { $_.Path })) $undoFiles }
  # The pathspec commits these files only, whatever else may have been staged during the prompts.
  Invoke-GitStep (@('commit', '-m', "chore(release): $tag", '--') + $releasePaths) $undoFiles
}
# --keep (not --hard) drops the release commit but keeps any edits made to other files during the prompts.
$undoCommit = @(if ($bump) { 'git reset --keep HEAD~1' })
Invoke-GitStep @('tag', '-a', $tag, '-m', "D2BS $tag") $undoCommit
Say "  > git push --atomic origin master $tag"
if (-not $DryRun) {
  # One atomic push: master and the tag both land or neither does.
  & git -C $repo push --atomic origin master $tag
  if ($LASTEXITCODE) {
    Bad "push failed (atomic: neither master nor $tag reached origin)"
    # The undo drops the release commit, and with it any edits made to CHANGELOG.md at the review.
    if ($bump) { $releaseSha = Read-Git @('rev-parse', '--short', 'HEAD'); Say "The release commit keeps the CHANGELOG.md you reviewed: git show ${releaseSha}:CHANGELOG.md" }
    Say 'Undo the local release, update, and release again:'
    @($undoCommit) + @("git tag -d $tag", 'git pull origin master', 'pwsh -File build\Release-D2BS.ps1') | ForEach-Object { Say "  $_" }
    exit 1
  }
  Ok "pushed master and $tag"
}

# === 7. After the push ==============================================================================================
# The next steps come before the optional watch, so stopping the watch with Ctrl+C cannot hide them.
$url = if ($slug) { "https://github.com/$slug" } else { '<origin>' }
Say ''
Say ('-' * 60)
Ok "$(if ($DryRun) { 'Dry run complete' } else { 'Release pushed' }): $tag"
Say ''
Say 'Next steps:'
$repoArg = if ($slug) { $slug } else { '<owner>/<repo>' }
Say '  1. Wait for the build to go green:'
Say "     $url/actions/workflows/build.yml"
if ($isCandidate) {
  Say "  2. It publishes the pre-release `"D2BS $tag`" (D2BS.dll, D2BS.pdb, D2BS-$tag.zip, the notes): $url/releases/tag/$tag"
  Say "  3. Give the testers its D2BS-$tag.zip (to extract into a kolbot folder) or its D2BS.dll; the D2BS log and"
  Say "     version() call it $($chosen -replace '-', 'U-')."
  Say '  4. Fixes for what they find add their entries to changelog.d. Then run this again: for the next release'
  Say '     candidate, or for the final once a release candidate holds up.'
  Say '  If the build fails on a download, re-run it from its Actions page. If the code or the DLL is at fault and no'
  Say '  tester has the DLL yet:'
  Say "     gh release delete $tag --repo $repoArg --yes   (if the release exists)"
} else {
  Say "  2. Review the DRAFT release `"D2BS $tag`" (D2BS.dll, D2BS.pdb, D2BS-$tag.zip, the notes): $url/releases"
  Say "  3. Try the DLL on a test install, e.g. with one D2Bot profile (D2BS-$tag.zip extracts into a kolbot folder)."
  Say '  4. Publish it and mark it Latest:'
  Say "     gh release edit $tag --repo $repoArg --draft=false --latest"
  Say '  If the build fails on a download, re-run it from its Actions page. If the code or the DLL is at fault:'
  Say "     gh release delete $tag --repo $repoArg --yes   (if the draft exists)"
}
Say "     git push origin :refs/tags/$tag"
Say "     git tag -d $tag"
Say "  then fix master and run this script again: it offers $chosen again."
# Testers report against the name, so a release candidate they have keeps its one DLL.
if ($isCandidate) { Say '  Once testers have it, cut the next release candidate instead.' }
if ($slug -and $hasGh -and -not $DryRun -and (Confirm-Step 'Watch the release build now?' $true -EofMeansNo)) {
  # The run for this tag's commit: a tag pushed again after a failed build also has the old tag's run.
  $tagCommit = Read-Git @('rev-parse', "$tag^{commit}")
  $runId = $null
  $ghFailures = 0
  # Up to a minute for the run to appear; three gh failures in a row (not one passing blip) end the wait.
  for ($try = 0; $try -lt 20 -and -not $runId -and $ghFailures -lt 3; $try++) {
    Start-Sleep -Seconds 3
    $runs = & gh run list --repo $slug --workflow build.yml --limit 20 --json databaseId,headBranch,headSha 2>$null
    if ($LASTEXITCODE) { $ghFailures++; continue }
    $ghFailures = 0
    if ($runs) {
      $runId = @($runs | ConvertFrom-Json | Where-Object { $_.headBranch -eq $tag -and $_.headSha -eq $tagCommit }) |
        Select-Object -First 1 -ExpandProperty databaseId
    }
  }
  if ($runId) { & gh run watch $runId --repo $slug }
  elseif ($ghFailures -ge 3) { Warn "gh could not list $slug's runs (check gh auth status); see $url/actions" }
  else { Warn "the release build has not appeared yet; see $url/actions" }
}
