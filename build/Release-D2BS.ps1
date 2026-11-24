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
  When master has moved on since the last release candidate, it also offers that candidate's final made from the
  candidate's own code: the release commit goes on the candidate's commit and takes the tag, and master gets the same
  VERSION and final section as a merge of it, its [Unreleased] entries kept for the next release (the release
  commit's CHANGELOG.md has [Unreleased] empty).

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
# A commit's parents and subject from the commit object itself, or $null when the object is missing: a shallow clone
# shows a merge at its boundary with no parents, and a message body can quote another commit's subject.
function Get-CommitInfo([string]$Commit) {
  $raw = Read-Git @('cat-file', 'commit', $Commit)
  if ($null -eq $raw) { return $null }
  $header, $message = "$raw" -split "`n`n", 2
  [pscustomobject]@{
    Parents = @([regex]::Matches($header, '(?m)^parent ([0-9a-f]+)$') | ForEach-Object { $_.Groups[1].Value })
    # As git log's %s reads it: the message's first paragraph, its lines joined with spaces.
    Subject = (@(("$message" -split "`n`n", 2)[0] -split "`n" | ForEach-Object { $_.Trim() } | Where-Object { $_ }) -join ' ')
  }
}
# What is wrong with a promotion's commit, or $null: it must sit on these parents with this subject (the "again"
# lookup finds it by that subject), change CHANGELOG.md and VERSION from $Base and nothing else, and hold those files
# exactly as approved. A commit hook may stage, rewrite or rename; this checks what was committed, in git's own terms,
# against the working files, and those files against the approved bytes.
function Get-ReleaseCommitProblem([string]$Dir, [string]$Commit, [string]$Base, [string[]]$Parents, [string]$Message, [byte[]]$Changelog, [byte[]]$Version) {
  $info = Get-CommitInfo $Commit
  if (-not $info -or ($info.Parents -join ' ') -cne ($Parents -join ' ')) { return 'it is not on the expected parents' }
  if ($info.Subject -cne $Message) { return "its subject is '$($info.Subject)', not '$Message' (a commit-msg hook?)" }
  $changed = ((Read-Git @('diff', '--name-only', $Base, $Commit)) -split "`n" | Sort-Object) -join ','
  if ($changed -cne 'CHANGELOG.md,VERSION') { return "it changes $changed, not just CHANGELOG.md and VERSION (a commit hook?)" }
  if (-not (Test-SameBytes (Join-Path $Dir 'CHANGELOG.md') $Changelog) -or -not (Test-SameBytes (Join-Path $Dir 'VERSION') $Version)) {
    return 'CHANGELOG.md or VERSION changed during the commit (a commit hook?)'
  }
  & git -C $Dir diff --quiet $Commit -- VERSION CHANGELOG.md 2>$null
  if ($LASTEXITCODE) { return 'it holds a CHANGELOG.md or VERSION other than the one approved (a commit hook?)' }
  $null
}
# The release commit of a final made from a release candidate's code: VERSION and CHANGELOG.md committed on the
# candidate's commit with plain git add and git commit (signing, hooks and line endings as any commit gets them), in a
# temporary worktree checked out at the candidate, so its own .gitattributes, hooks and files apply. Returns its SHA;
# a failure prints how to undo and stops.
function New-CandidateReleaseCommit([string]$Candidate, [string]$Version, [string]$Changelog, [string]$Message, [string[]]$Undo) {
  $dir = Join-Path ([IO.Path]::GetTempPath()) "d2bs-release-$([guid]::NewGuid().ToString('N').Substring(0, 12))"
  function Stop-Here([string]$Why) { Bad $Why; Say 'To undo:'; $Undo | ForEach-Object { Say "  $_" }; exit 1 }
  $bytes = [Text.UTF8Encoding]::new($false).GetBytes($Changelog)
  try {
    # Inside the try: git keeps a worktree whose post-checkout hook failed. Out-Host: git's own output must not join
    # the SHA this function returns.
    Invoke-GitStep @('worktree', 'add', '--quiet', '--detach', $dir, $Candidate) $Undo | Out-Host
    $versionBytes = [Text.UTF8Encoding]::new($false).GetBytes("$Version`n")
    [IO.File]::WriteAllBytes((Join-Path $dir 'VERSION'), $versionBytes)
    [IO.File]::WriteAllBytes((Join-Path $dir 'CHANGELOG.md'), $bytes)
    foreach ($step in @(@('add', '--', 'VERSION', 'CHANGELOG.md'), @('commit', '--quiet', '-m', $Message))) {
      Say ('  > git -C <temporary worktree> ' + (($step | ForEach-Object { if ($_ -match '\s') { "`"$_`"" } else { $_ } }) -join ' '))
      & git -C $dir @step | Out-Host
      if ($LASTEXITCODE) { Stop-Here "git $($step[0]) failed ($LASTEXITCODE)" }
    }
    $sha = Read-Git @('-C', $dir, 'rev-parse', 'HEAD')
    $problem = if ($sha) { Get-ReleaseCommitProblem $dir $sha $Candidate @($Candidate) $Message $bytes $versionBytes } else { 'git made no commit' }
    if ($problem) { Stop-Here "the release commit is wrong: $problem" }
    $sha
  } finally {
    & git -C $repo worktree remove --force $dir 2>$null | Out-Null
    $removed = -not $LASTEXITCODE
    # git can drop the registration and still fail to delete a folder Windows holds open.
    if (Test-Path -LiteralPath $dir) {
      $registered = "$(Read-Git @('worktree', 'list', '--porcelain'))".Contains($dir.Replace('\', '/'))
      $how = if ($removed -or -not $registered) { 'delete that folder, then run git worktree prune' } else { "run git worktree remove --force `"$dir`"" }
      Warn "could not remove the temporary worktree at ${dir}: $how"
    }
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
# The branch too: a switch to another branch at the same commit would take the release commit there, while the push
# names master.
$refAtStart = Read-Git @('symbolic-ref', '--quiet', 'HEAD')
function Test-HeadMoved { (Read-Git @('rev-parse', 'HEAD')) -ne $headAtStart -or (Read-Git @('symbolic-ref', '--quiet', 'HEAD')) -cne $refAtStart }

$status = @((Read-Git @('status', '--porcelain', '--untracked-files=all')) -split "`n" | Where-Object { $_ })
$tracked = @($status | Where-Object { -not $_.StartsWith('??') })
# changelog.d's own files are named by its checks below, which stop a release on an uncommitted one.
$untracked = @($status | Where-Object { $_.StartsWith('??') } | ForEach-Object { $_.Substring(3) } | Where-Object { -not $_.StartsWith('changelog.d/') })
if ($tracked.Count) { Fail "the working tree has $($tracked.Count) tracked change(s); the release commit must hold only VERSION, CHANGELOG.md and the changelog.d files it folds in" }
else { Ok 'no tracked changes' }
if ($untracked.Count) { Warn "untracked files (left out of the release): $(($untracked | Select-Object -First 5) -join ', ')$(if ($untracked.Count -gt 5) { ', ...' })" }
# A merge whose tree equals HEAD leaves git status clean, yet no release commit belongs inside it (git commit refuses
# paths during a merge, and a promotion's own merge would replace it).
if ($null -ne (Read-Git @('rev-parse', '--quiet', '--verify', 'MERGE_HEAD'))) { Fail 'a merge is in progress: commit it or run git merge --abort first' }

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
# Bytes read as Read-TextFile reads a file: UTF-8, without a byte order mark.
function ConvertFrom-Utf8Bytes([byte[]]$Bytes) {
  $text = [Text.UTF8Encoding]::new($false).GetString($Bytes)
  if ($text.Length -and $text[0] -eq [char]0xFEFF) { $text.Substring(1) } else { $text }
}
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
# Commits since the latest release tag that are not release commits: what a final would ship untested when that tag
# is a release candidate's.
$untested = @($log | Where-Object { $_ -notmatch '^\S+ chore\(release\): ' })
# A final can also be made from the last release candidate's own commit, leaving out what master gained since: only
# while VERSION is that candidate, its tag is the latest and on master's history, and master has moved on. Counted by
# commit, merges included, since a subject can't tell what a commit changed.
$promoteFrom = $null
if ((Test-Candidate $currentVersion) -and $latestTag -ceq "v$current") {
  $rcCommit = Read-Git @('rev-parse', '--verify', '--quiet', "$latestTag^{commit}")
  $past = if ($rcCommit) { Read-Git @('rev-list', '--count', "$rcCommit..HEAD") } else { $null }
  if ($past -match '^\d+$' -and [int]$past -gt 0 -and $null -ne (Read-Git @('merge-base', '--is-ancestor', $rcCommit, 'HEAD')) -and
      (Read-Git @('show', "${rcCommit}:VERSION")) -ceq $current) {
    $promoteFrom = [pscustomobject]@{ Tag = $latestTag; Commit = $rcCommit; Short = (Read-Git @('rev-parse', '--short', $rcCommit)); Count = [int]$past }
  }
  # The candidate testers have is origin's tag: git fetch won't move a local tag that differs (the candidate released
  # again from another clone), so a stale one here would promote other code.
  if ($promoteFrom -and $NoFetch) { Warn "-NoFetch: $latestTag is not compared with origin's" }
  elseif ($promoteFrom) {
    $remoteTag = Read-Git @('ls-remote', '--tags', 'origin', "refs/tags/$latestTag", "refs/tags/$latestTag^{}")
    $remoteLines = @("$remoteTag" -split "`n" | Where-Object { $_ })
    $peeled = @($remoteLines | Where-Object { $_.EndsWith('^{}') }) + @($remoteLines | Where-Object { -not $_.EndsWith('^{}') })
    $remoteCommit = if ($peeled.Count) { ($peeled[0] -split '\s+')[0] } else { $null }
    if ($remoteCommit -cne $rcCommit) {
      $why = if ($null -eq $remoteTag) { 'could not ask origin about it' } elseif (-not $remoteCommit) { 'origin has no such tag' } else { "origin's names $($remoteCommit.Substring(0, 7)): git fetch --tags --force, then run this again" }
      Warn "no final of $latestTag's code is offered: $why"
      $promoteFrom = $null
    }
  }
}
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
$finalChoice = $null
# The same version again only when its release commit exists but its tag is gone (deleted after a failed build).
$currentTag = "v$current"
# Release commits by their exact subject (a merge message can quote one in its body). A final made from a release
# candidate's code has two: the one off master, which the tag names, and master's merge of it, which --topo-order
# lists first.
$releaseSubject = "chore(release): v$current"
$releaseShas = @(@((Read-Git @('log', '--topo-order', '--format=%H %s', '--extended-regexp', "--grep=^chore\(release\): v$($current -replace '\.', '\.')$")) -split "`n") |
    Where-Object { $_ -and ($_ -split ' ', 2)[1] -ceq $releaseSubject } | ForEach-Object { ($_ -split ' ', 2)[0] })
$releaseCommit = if ($releaseShas.Count) { Read-Git @('rev-parse', '--short', $releaseShas[0]) } else { $null }
$sameAgain = [bool]($releaseCommit -and -not (Test-LocalTag $currentTag) -and -not (Test-RemoteTag $currentTag))
# What "again" tags: HEAD, or the release commit off master when the one on master is the merge of it, read from the
# commit objects themselves.
$againTarget = $null
$againBlocked = $false
if ($sameAgain) {
  $info = Get-CommitInfo $releaseShas[0]
  # Only a final is ever made from a candidate's code, so a release candidate's merge is an ordinary one.
  if ($info -and $info.Parents.Count -ge 2 -and -not (Test-Candidate $currentVersion)) {
    $second = Get-CommitInfo $info.Parents[1]
    if ($null -eq $second) {
      # Tagging HEAD here would publish master's code under the final of a release candidate's, so only "again" goes;
      # a new version needs nothing from that commit.
      Warn "the release commit $releaseCommit is a merge whose second parent is missing here (a shallow clone?): to release $current again, fetch the whole history first"
      $sameAgain = $false
      $againBlocked = $true
    } elseif ($second.Subject -ceq $releaseSubject) {
      $againTarget = [pscustomobject]@{ Commit = $info.Parents[1]; Short = (Read-Git @('rev-parse', '--short', $info.Parents[1])) }
    }
  }
  if ($sameAgain) { $options += [pscustomobject]@{ Label = "$current again (release commit $releaseCommit, tag deleted)"; Version = $current; Promote = $false } }
}
if (Test-Candidate $currentVersion) {
  # Inside a release candidate cycle: the next candidate, or the final of the version they lead to.
  $final = '{0}.{1}.{2}' -f $currentVersion.Major, $currentVersion.Minor, $currentVersion.Patch
  $rc = [int]($currentVersion.PreReleaseLabel -replace '^rc\.', '')
  $options += [pscustomobject]@{ Label = 'next release candidate'; Version = "$final-rc.$($rc + 1)"; Promote = $false }
  $options += [pscustomobject]@{ Label = 'final'; Version = $final; Promote = $false }
  $finalChoice = $options.Count
  if ($promoteFrom) {
    $options += [pscustomobject]@{ Label = "final of $($promoteFrom.Tag)'s code (leaves out the $($promoteFrom.Count) commit(s) since)"; Version = $final; Promote = $true }
  }
} else {
  $next = [ordered]@{
    patch = '{0}.{1}.{2}' -f $currentVersion.Major, $currentVersion.Minor, ($currentVersion.Patch + 1)
    minor = '{0}.{1}.0' -f $currentVersion.Major, ($currentVersion.Minor + 1)
    major = '{0}.0.0' -f ($currentVersion.Major + 1)
  }
  foreach ($kind in $next.Keys) { $options += [pscustomobject]@{ Label = "$kind release candidate"; Version = "$($next[$kind])-rc.1"; Promote = $false } }
  foreach ($kind in $next.Keys) { $options += [pscustomobject]@{ Label = $kind; Version = $next[$kind]; Promote = $false } }
}
for ($i = 0; $i -lt $options.Count; $i++) { Say ("  {0}) {1} -> v{2}" -f ($i + 1), $options[$i].Label, $options[$i].Version) }
# Inside a cycle with no new entries, another release candidate would have no notes, so the final is the default.
$default = if ((Test-Candidate $currentVersion) -and -not $groups.Count -and -not $sameAgain) { $finalChoice } else { 1 }
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
  if ($pick.Promote -and $candidate -cne $pick.Version) { Warn "the final of $($promoteFrom.Tag)'s code is $($pick.Version)"; continue }
  if ($againBlocked -and $candidate -ceq $current) { Warn "$current again needs its release commit off master, which is missing here: fetch the whole history first"; continue }
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
# The "again" target belongs to the version offered again; a new version chosen beside it makes its own commit.
if ($bump) { $againTarget = $null }
$isCandidate = Test-Candidate (ConvertTo-Version $chosen)
# A final made from the last release candidate's code: two commits (see the plan), and [Unreleased] stays on master.
$promote = [bool]$pick.Promote
# A final merges the release candidates' sections into its own; a release candidate dates [Unreleased] alone.
$merging = @(if (-not $isCandidate) { $candidates })
# The tag build posts the version's CHANGELOG.md section as the release notes and fails without one.
$date = Get-Date -Format 'yyyy-MM-dd'
# A block left open inside the version's section takes the rest of the file into its notes.
$unclosedNotes = "an HTML comment or a code fence in the [$chosen] notes never closes, so they would run to the end of CHANGELOG.md"
$newChangelog = $null
# A promotion's release commit's CHANGELOG.md, and master's as bytes, as approved at the review.
$releaseChangelog = $null
$approvedBytes = $null
$approvedVersion = $null
# A re-release commits nothing, so it leaves changelog.d alone, and so does a promotion, whose entries describe code
# the release leaves out.
$folded = @(if ($bump -and -not $promote) { $fragments.Entries })
if ($promote) {
  try { $newChangelog = Update-ChangelogForRelease -Text $changelog -Version $chosen -Date $date -Tag $tag -KeepUnreleased }
  catch { Fail $_.Exception.Message }
  # The notes the tag build reads, from the CHANGELOG.md the release commit holds.
  $notes = if ($newChangelog) { Get-ChangelogSection -Text (Clear-ChangelogUnreleased $newChangelog) -Name $chosen } else { $null }
  if ($notes -and (Test-ChangelogUnclosed $notes)) { Fail $unclosedNotes }
} elseif ($bump) {
  if ($fragments.Problems.Count) {
    Fail "changelog.d has $($fragments.Problems.Count) problem(s), listed above: fix them, or move those files out of changelog.d"
  }
  try { $newChangelog = Update-ChangelogForRelease -Text $assembled -Version $chosen -Date $date -Tag $tag }
  catch { Fail $_.Exception.Message }
  $notes = if ($newChangelog) { Get-ChangelogSection -Text $newChangelog -Name $chosen } else { $unreleased }
  if (Test-ChangelogUnclosed $notes) { Fail $unclosedNotes }
} else {
  # The tag build reads the tagged commit's CHANGELOG.md, which for a release commit off master is that commit's own.
  $taggedChangelog = if ($againTarget) { "$(Read-Git @('show', "$($againTarget.Commit):CHANGELOG.md"))" } else { $changelog }
  $notes = Get-ChangelogSection -Text $taggedChangelog -Name $chosen
  if ($null -eq $notes) {
    $cause = if (Test-ChangelogHeadingHidden $taggedChangelog $chosen) { ' (it is inside an HTML comment or a code fence)' } else { '' }
    Fail "CHANGELOG.md has no [$chosen] heading${cause}: the release would have no notes"
  } elseif (Test-ChangelogUnclosed $notes) { Fail $unclosedNotes }
  elseif (-not (Test-ChangelogEntries $notes)) { Fail "CHANGELOG.md has no entries under [$chosen]: the release would have no notes" }
  # A fix made for a re-release belongs to the version it fixes, which is the section the release posts. Master's
  # entries never reach a release commit off master, so they wait for the next release.
  $waiting = @(if (-not $againTarget -and (Test-ChangelogEntries (Get-ChangelogSection -Text $changelog -Name 'Unreleased'))) { '[Unreleased]' }) +
    @(if (-not $againTarget -and $fragments.Entries.Count) { 'changelog.d' })
  if ($waiting.Count) {
    Warn "$($waiting -join ' and ') $(if ($waiting.Count -gt 1) { 'have' } else { 'has' }) entries, and re-releasing $tag posts only [$chosen]: move a fix's entry there first"
  }
}
# A final is meant to be the code its last release candidate's testers ran; its own release commit (there when it is
# released again) changes only VERSION and CHANGELOG.md. A promotion, or its tag again, is that code.
if (-not $isCandidate -and -not $promote -and -not $againTarget -and $latestTag -and (Test-Candidate (ConvertTo-Version $latestTag.Substring(1))) -and $untested.Count) {
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
if ($promote) {
  Step "write VERSION: $current -> $chosen"
  $taken = if ($merging.Count -gt 1) { "merge CHANGELOG.md's $mergedNames sections into" } else { "rename CHANGELOG.md's $mergedNames section to" }
  Step "$taken [$chosen] - $date, below [Unreleased], whose entries stay for the next release"
  Step 'review the change; CHANGELOG.md can be edited there'
  Step "commit VERSION and CHANGELOG.md (its [Unreleased] emptied) on $($promoteFrom.Tag)'s commit $($promoteFrom.Short) as `"chore(release): $tag`": the code its testers ran, without the $($promoteFrom.Count) commit(s) since"
  Step "commit VERSION and CHANGELOG.md on master as `"chore(release): $tag`", a merge of that commit"
} elseif ($bump) {
  Step "write VERSION: $current -> $chosen"
  if ($folded.Count) { Step "add the entries of $foldedFiles to CHANGELOG.md's [Unreleased] section" }
  if ($merging.Count) { Step "merge CHANGELOG.md's $mergedNames and [Unreleased] sections into [$chosen] - $date" }
  else { Step "date CHANGELOG.md's Unreleased section: [$chosen] - $date" }
  Step 'review the change; CHANGELOG.md can be edited there'
  if ($folded.Count) { Step "commit VERSION and CHANGELOG.md, deleting $foldedFiles, as `"chore(release): $tag`"" }
  else { Step "commit VERSION and CHANGELOG.md as `"chore(release): $tag`"" }
} elseif ($againTarget) {
  Step "VERSION already says ${chosen}: the tag goes on the release commit $($againTarget.Short), the one off master that its merge $releaseCommit brought in"
} else {
  Step "VERSION already says ${chosen}: the tag goes on HEAD ($(Read-Git @('log', '-1', '--format=%h %s')))"
}
Step "annotated tag $tag$(if ($promote) { ' on the release commit made from the candidate''s code' })"
$built = if ($isCandidate) { 'builds and publishes a pre-release' } else { 'builds a DRAFT release' }
Step "push master and $tag in one atomic push; the tag $built"
if ($notes) {
  $withFolded = if ($folded.Count) { " with $foldedFiles" } else { '' }
  $source = if (-not $bump) { "[$chosen]" } elseif ($promote) { "$mergedNames$(if ($merging.Count -gt 1) { ', merged' })" } elseif ($merging.Count) { "[Unreleased]$withFolded and $mergedNames, merged" } else { "[Unreleased]$withFolded" }
  Say "  Release notes (CHANGELOG.md ${source}):"
  $notes -split "`n" | ForEach-Object { Say "    $_" }
  foreach ($line in Get-ChangelogWrappedLines $notes) { Warn "a wrapped line shows as its own line on GitHub; join it to its entry: $($line.Trim())" }
}
Say ('-' * 60)
if (-not (Confirm-Step 'Proceed?' $true)) { Stop-Release }
# The new CHANGELOG.md was made from the file as read at the start, and the plan named the commit to tag: an edit or
# a commit made during the prompts would be lost or published unseen.
if (-not $DryRun -and ((Read-Changelog) -cne $changelog -or (Read-Git @('status', '--porcelain', '--', 'VERSION', 'CHANGELOG.md')) -or
    (Test-HeadMoved) -or ($bump -and -not $promote -and @(Get-FragmentChanges).Count))) {
  Stop-Release 'VERSION, CHANGELOG.md, changelog.d or HEAD changed during the prompts: run this again'
}

# === 5. Prepare and review ==========================================================================================
if ($bump) {
  Say ''
  Say 'Preparing'
  Say "  > write VERSION: $chosen"
  $intoUnreleased = if ($folded.Count) { "[Unreleased] with $foldedFiles" } else { '[Unreleased]' }
  if ($promote) { Say "  > write CHANGELOG.md: $mergedNames -> [$chosen] - $date, [Unreleased] kept" }
  elseif ($merging.Count) { Say "  > write CHANGELOG.md: $mergedNames and $intoUnreleased -> [$chosen] - $date" }
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
      if ($promote) { Say "  Master's commit takes CHANGELOG.md as it is then; the release commit takes it with [Unreleased] emptied." }
      while ($true) {
        $answer = (Ask '(y) commit + tag + push  (d) show the diff  (n) abort: ').ToLower()
        if ($answer -in 'y', 'yes') {
          # A commit made while this prompt waited would go out on top of the release unseen.
          if (Test-HeadMoved) { Stop-Release 'HEAD changed during the review: run this again' }
          # The commit takes the files as they are now, edits included; the tag build needs the version's notes.
          $versionNow = if (Test-Path -LiteralPath $versionFile) { (Read-TextFile $versionFile).Trim() } else { '' }
          if ($versionNow -cne $chosen) { Warn "VERSION no longer says ${chosen}: put it back, or answer n"; continue }
          $changelogNow = if (Test-Path -LiteralPath $changelogFile) { Read-Changelog } else { '' }
          # A promotion's tag build reads the release commit's CHANGELOG.md, which is this one with [Unreleased] emptied.
          # Its commits take these bytes, read once here, so an edit saved after this answer reaches neither commit.
          if ($promote) {
            # Plain assignments: an if expression would hand an empty file's empty array back as $null.
            $approvedBytes = [byte[]]@()
            if (Test-Path -LiteralPath $changelogFile) { $approvedBytes = [IO.File]::ReadAllBytes($changelogFile) }
            $approvedVersion = [byte[]]@()
            if (Test-Path -LiteralPath $versionFile) { $approvedVersion = [IO.File]::ReadAllBytes($versionFile) }
            if ((ConvertFrom-Utf8Bytes $approvedVersion).Trim() -cne $chosen) { Warn "VERSION no longer says ${chosen}: put it back, or answer n"; continue }
            $changelogNow = ConvertFrom-Utf8Bytes $approvedBytes
            # Master's copy keeps [Unreleased], and a block left open there would hide [$chosen] on master alone.
            if ($null -eq (Get-ChangelogSection -Text $changelogNow -Name $chosen) -and (Test-ChangelogHeadingHidden $changelogNow $chosen)) {
              Warn "CHANGELOG.md's [$chosen] heading is inside an HTML comment or a code fence now (on master's copy): close it, or answer n"; continue
            }
            try { $changelogNow = Clear-ChangelogUnreleased $changelogNow }
            catch { Warn "CHANGELOG.md has no [Unreleased] heading now, whose entries the release commit leaves out: put it back, or answer n"; continue }
            $releaseChangelog = $changelogNow
          }
          $notesNow = Get-ChangelogSection -Text $changelogNow -Name $chosen
          if ($null -eq $notesNow) {
            $cause = if (Test-ChangelogHeadingHidden $changelogNow $chosen) { ' (it is inside an HTML comment or a code fence)' } else { '' }
            Warn "CHANGELOG.md has no [$chosen] heading now${cause}: put it back, or answer n"; continue
          }
          if (Test-ChangelogUnclosed $notesNow) { Warn "$unclosedNotes (close it, or answer n)"; continue }
          if (-not (Test-ChangelogEntries $notesNow)) { Warn "CHANGELOG.md has no entries under [$chosen] now: the release would have no notes"; continue }
          if ($promote) { $approved = $true; break }
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
# The tag's runs before this push: a release commit tagged again has the same tag and commit as its failed run. Listed
# before anything is committed, so a slow GitHub API never holds up the steps between the commit and the push.
$earlierRuns = @()
$earlierKnown = $false
if ($slug -and $hasGh -and -not $DryRun) {
  $listed = & gh run list --repo $slug --workflow build.yml --limit 50 --json databaseId,headBranch 2>$null
  if (-not $LASTEXITCODE) {
    $earlierKnown = $true
    if ($listed) { $earlierRuns = @($listed | ConvertFrom-Json | Where-Object { $_.headBranch -eq $tag } | ForEach-Object { $_.databaseId }) }
  }
}
Say ''
Say 'Committing, tagging, pushing'
$tagTarget = @()
if ($promote) {
  $undoFiles = @('git restore --staged --worktree -- VERSION CHANGELOG.md')
  # --quit, not --abort: abort's reset would also undo anything else staged meanwhile, which the check below reports.
  $undoMerge = @('git merge --quit') + $undoFiles
  if (-not $DryRun) {
    Say "  Should these steps stop before master's commit, this undoes them: git merge --quit (if a merge is under way), then $($undoFiles[0])"
    Say "  After master's commit: git reset --keep HEAD~1, and git tag -d $tag once the tag exists"
  }
  # Stops the promotion. A CHANGELOG.md that is no longer what the review approved is copied out first, since the undo
  # puts back the committed one.
  function Stop-Changed([string]$Message, [string[]]$Undo) {
    if ((Test-Path -LiteralPath $changelogFile) -and -not (Test-SameBytes $changelogFile $approvedBytes)) {
      $kept = Join-Path ([IO.Path]::GetTempPath()) "d2bs-CHANGELOG-$(Get-Date -Format 'yyyyMMdd-HHmmss')-$PID.md"
      Copy-Item -LiteralPath $changelogFile -Destination $kept
      Write-Host "Your edits to CHANGELOG.md are kept in $kept"
    }
    Bad $Message; Say 'To undo:'; $Undo | ForEach-Object { Say "  $_" }; exit 1
  }
  function Get-Changed {
    if (Test-HeadMoved) { 'HEAD' } elseif (-not (Test-SameBytes $changelogFile $approvedBytes)) { 'CHANGELOG.md' }
    elseif (-not (Test-SameBytes $versionFile $approvedVersion)) { 'VERSION' }
  }
  $promotedSha = '<the release commit>'
  if ($DryRun) {
    Say "  > commit VERSION and CHANGELOG.md (its [Unreleased] emptied) on $($promoteFrom.Tag)'s commit $($promoteFrom.Short), in a temporary worktree, as `"chore(release): $tag`""
  } else {
    $promotedSha = New-CandidateReleaseCommit $promoteFrom.Commit $chosen $releaseChangelog "chore(release): $tag" $undoFiles
    # master's commit takes the files from the disk, which must still be what the review approved, on the branch and
    # commit it checked. The release commit made so far is only an object nothing names yet.
    $moved = Get-Changed
    if ($moved) { Stop-Changed "$moved changed after the review's answer: run this again" $undoFiles }
  }
  # master's commit: a merge that keeps master's tree (-s ours), then VERSION and CHANGELOG.md, committed as any commit.
  # --no-autostash: merge.autoStash would stash the reviewed files and stage the old ones.
  Invoke-GitStep @('merge', '--quiet', '--no-ff', '--no-commit', '--no-autostash', '-s', 'ours', $promotedSha) $undoMerge
  Invoke-GitStep @('add', '--', 'VERSION', 'CHANGELOG.md') $undoMerge
  if (-not $DryRun) {
    # The merge commit takes the whole index (git commit refuses paths in a merge): exactly the two files, as approved.
    $staged = ((Read-Git @('diff', '--cached', '--name-only')) -split "`n" | Where-Object { $_ } | Sort-Object) -join ','
    if ($staged -cne 'CHANGELOG.md,VERSION') { Stop-Changed "master's commit would hold $(if ($staged) { $staged } else { 'no change' }), not just CHANGELOG.md and VERSION" $undoMerge }
    $moved = Get-Changed
    if ($moved) { Stop-Changed "$moved changed after the review's answer: run this again" $undoMerge }
  }
  Invoke-GitStep @('commit', '--quiet', '-m', "chore(release): $tag") $undoMerge
  if (-not $DryRun) {
    $problem = Get-ReleaseCommitProblem $repo (Read-Git @('rev-parse', 'HEAD')) $headAtStart @($headAtStart, $promotedSha) "chore(release): $tag" $approvedBytes $approvedVersion
    # The restore first: git reset --keep refuses while a file it resets has edits (a post-commit hook's, say).
    if ($problem) { Stop-Changed "master's release commit is wrong: $problem" @('git restore -- VERSION CHANGELOG.md', 'git reset --keep HEAD~1') }
  }
  $tagTarget = @($promotedSha)
} elseif ($againTarget) {
  $tagTarget = @($againTarget.Commit)
} elseif ($bump) {
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
Invoke-GitStep (@('tag', '-a', $tag, '-m', "D2BS $tag") + $tagTarget) $undoCommit
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
  if ($promote) { Say "  master says VERSION $chosen now; its [Unreleased] entries and changelog.d files wait for the next release." }
  Say '  If the build fails on a download, re-run it from its Actions page. If the code or the DLL is at fault:'
  Say "     gh release delete $tag --repo $repoArg --yes   (if the draft exists)"
}
# A promotion's tag stays: a fix on master can't reach its release commit, master's [Unreleased] link starts at it, and
# the next version follows it.
if ($promote -or $againTarget) {
  Say "  then keep $tag (master's [Unreleased] link starts there), fix master and release the next version."
} else {
  Say "     git push origin :refs/tags/$tag"
  Say "     git tag -d $tag"
  Say "  then fix master and run this script again: it offers $chosen again."
}
# Testers report against the name, so a release candidate they have keeps its one DLL.
if ($isCandidate) { Say '  Once testers have it, cut the next release candidate instead.' }
# A tag pushed again can name the commit its failed run built, so without the runs from before the push the two can't
# be told apart.
if ($slug -and $hasGh -and -not $DryRun -and -not $bump -and -not $earlierKnown) {
  Warn "gh could not list the earlier runs of $tag before the push, so this one can't be told from them; see $url/actions"
} elseif ($slug -and $hasGh -and -not $DryRun -and (Confirm-Step 'Watch the release build now?' $true -EofMeansNo)) {
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
      $runId = @($runs | ConvertFrom-Json | Where-Object { $_.headBranch -eq $tag -and $_.headSha -eq $tagCommit -and $_.databaseId -notin $earlierRuns }) |
        Select-Object -First 1 -ExpandProperty databaseId
    }
  }
  if ($runId) { & gh run watch $runId --repo $slug }
  elseif ($ghFailures -ge 3) { Warn "gh could not list $slug's runs (check gh auth status); see $url/actions" }
  else { Warn "the release build has not appeared yet; see $url/actions" }
}
