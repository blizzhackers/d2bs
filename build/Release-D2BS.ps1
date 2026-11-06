#Requires -Version 7.2
<#
  Interactive release for D2BS. Checks that master is ready, proposes the next version: a release candidate (1.7.0-rc.1,
  then 1.7.0-rc.2) or a final (1.7.0), or the same version again when its release commit exists but its tag was
  deleted. It writes VERSION and updates CHANGELOG.md: a release candidate dates the Unreleased section as its own, and
  a final merges the release candidates' sections and Unreleased into one dated section. After a review (CHANGELOG.md
  can be edited there) it commits both as "chore(release): v<version>", tags it (annotated) and pushes master and the
  tag in one atomic push. The tag push makes the build workflow publish a release candidate as a GitHub pre-release,
  or create a final as a DRAFT release to publish after a test; each carries D2BS.dll and D2BS.pdb, and its notes are
  the version's CHANGELOG.md section.

    pwsh -File build\Release-D2BS.ps1             interactive
    pwsh -File build\Release-D2BS.ps1 -DryRun     rehearse: prints every step that would change something, changes nothing
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
# candidate, as a semantic version: 1.7.0-rc.2 sorts after 1.7.0-rc.1 and before 1.7.0.
function ConvertTo-Version([string]$Text) {
  if ($Text -notmatch '^(\d{1,5})\.(\d{1,5})\.(\d{1,5})(-rc\.(\d{1,5}))?$') { return $null }
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
$untracked = @($status | Where-Object { $_.StartsWith('??') } | ForEach-Object { $_.Substring(3) })
if ($tracked.Count) { Fail "the working tree has $($tracked.Count) tracked change(s); the release commit must hold only VERSION and CHANGELOG.md" }
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
if (-not $currentVersion -or $currentVersion.ToString() -ne $current) {
  Stop-Release "VERSION must hold major.minor.patch or major.minor.patch-rc.N without leading zeros, not '$current'"
}

function Read-Changelog { Read-TextFile $changelogFile }
if (-not (Test-Path -LiteralPath $changelogFile -PathType Leaf)) { Stop-Release 'CHANGELOG.md is missing: the release notes come from it' }
# The review and its undo work through git, which can't put back a file it doesn't track.
if (-not (Read-Git @('ls-files', '--', 'CHANGELOG.md'))) { Fail 'CHANGELOG.md is not committed: commit it to master first' }
$changelog = Read-Changelog
$unreleased = Get-ChangelogSection -Text $changelog -Name 'Unreleased'
if ($null -eq $unreleased) { Stop-Release 'CHANGELOG.md has no "## [Unreleased]" heading' }
if ($changelog -notmatch '(?m)^\[Unreleased\]: ') { Warn 'CHANGELOG.md has no [Unreleased] link at its end, so the release adds no compare link' }

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
if ($groups.Count) {
  Say "  CHANGELOG.md [Unreleased]: $(Format-EntryCounts $groups)"
  if ($groups.Contains('Added')) { Say '  It has Added entries: Semantic Versioning makes that a minor version.' }
} elseif ($candidates.Count) {
  Say '  CHANGELOG.md has no entries under [Unreleased]: a final can still merge the release candidates, but another candidate needs new entries there first'
} else {
  Warn 'CHANGELOG.md has no entries under [Unreleased]: a new version needs them there first'
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
$pick = $null
while (-not $pick) {
  $answer = Ask 'Choice [1]: '
  if (-not $answer) { $answer = '1' }
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
  # Newer than the latest release tag and than VERSION (the latest release, even one shipped untagged, like 1.6.4U);
  # only the "again" case may reuse VERSION's number.
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
$newChangelog = $null
if ($bump) {
  try { $newChangelog = Update-ChangelogForRelease -Text $changelog -Version $chosen -Date $date -Tag $tag }
  catch { Fail $_.Exception.Message }
  $notes = if ($newChangelog) { Get-ChangelogSection -Text $newChangelog -Name $chosen } else { $unreleased }
} else {
  $notes = Get-ChangelogSection -Text $changelog -Name $chosen
  if (-not (Test-ChangelogEntries $notes)) { Fail "CHANGELOG.md has no entries under [$chosen]: the release would have no notes" }
  # A fix made for a re-release belongs to the version it fixes, which is the section the release posts.
  if (Test-ChangelogEntries $unreleased) { Warn "[Unreleased] has entries, and re-releasing $tag posts only [$chosen]: move a fix's entry there first" }
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
if ($bump) {
  Say "    1. write VERSION: $current -> $chosen"
  if ($merging.Count) { Say "    2. merge CHANGELOG.md's $mergedNames and [Unreleased] sections into [$chosen] - $date" }
  else { Say "    2. date CHANGELOG.md's Unreleased section: [$chosen] - $date" }
  Say '    3. review the change; CHANGELOG.md can be edited there'
  Say "    4. commit VERSION and CHANGELOG.md as `"chore(release): $tag`""
} else {
  Say "    1. VERSION already says ${chosen}: the tag goes on HEAD ($(Read-Git @('log', '-1', '--format=%h %s')))"
}
Say "    $(if ($bump) { 5 } else { 2 }). annotated tag $tag"
$built = if ($isCandidate) { 'builds and publishes a pre-release' } else { 'builds a DRAFT release' }
Say "    $(if ($bump) { 6 } else { 3 }). push master and $tag in one atomic push; the tag $built"
if ($notes) {
  $source = if (-not $bump) { "[$chosen]" } elseif ($merging.Count) { "[Unreleased] and $mergedNames, merged" } else { '[Unreleased]' }
  Say "  Release notes (CHANGELOG.md ${source}):"
  $notes -split "`n" | ForEach-Object { Say "    $_" }
  foreach ($line in Get-ChangelogWrappedLines $notes) { Warn "a wrapped line shows as its own line on GitHub; join it to its entry: $($line.Trim())" }
}
Say ('-' * 60)
if (-not (Confirm-Step 'Proceed?' $true)) { Stop-Release }
# The new CHANGELOG.md was made from the file as read at the start, and the plan named the commit to tag: an edit or
# a commit made during the prompts would be lost or published unseen.
if (-not $DryRun -and ((Read-Changelog) -cne $changelog -or (Read-Git @('status', '--porcelain', '--', 'VERSION', 'CHANGELOG.md')) -or
    (Read-Git @('rev-parse', 'HEAD')) -ne $headAtStart)) {
  Stop-Release 'VERSION, CHANGELOG.md or HEAD changed during the prompts: run this again'
}

# === 5. Prepare and review ==========================================================================================
if ($bump) {
  Say ''
  Say 'Preparing'
  Say "  > write VERSION: $chosen"
  if ($merging.Count) { Say "  > write CHANGELOG.md: $mergedNames and [Unreleased] -> [$chosen] - $date" }
  else { Say "  > write CHANGELOG.md: [Unreleased] -> [$chosen] - $date" }
  Say ''
  Say 'Review'
  if ($DryRun) { Say '  (dry run: nothing written, no review)' }
  else {
    # Leaving the review any way but y (n, the end of piped input, Ctrl+C) puts both files back.
    $approved = $false
    try {
      [IO.File]::WriteAllText($versionFile, "$chosen`n")
      [IO.File]::WriteAllText($changelogFile, $newChangelog)
      & git -C $repo --no-pager diff --stat
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
          if ((Read-TextFile $versionFile).Trim() -ne $chosen) { Warn "VERSION no longer says ${chosen}: put it back, or answer n"; continue }
          if (-not (Test-ChangelogEntries (Get-ChangelogSection -Text (Read-Changelog) -Name $chosen))) {
            Warn "CHANGELOG.md has no entries under [$chosen] now: the release would have no notes"; continue
          }
          $approved = $true; break
        }
        if ($answer -eq 'd') { & git -C $repo --no-pager diff -- VERSION CHANGELOG.md; continue }
        if ($answer -in 'n', 'no') { Stop-Release }
        Warn 'enter y, d or n'
      }
    } finally {
      if (-not $approved) {
        # git restore copies from the index, so a commit made during the review that took the release files keeps
        # them: compare with the commit the release started from before saying they are back.
        & git -C $repo restore -- VERSION CHANGELOG.md
        $restored = -not $LASTEXITCODE
        & git -C $repo diff --quiet $headAtStart -- VERSION CHANGELOG.md
        if ($restored -and -not $LASTEXITCODE) { Write-Host "VERSION put back to $current, and CHANGELOG.md with it" }
        else {
          Write-Host ('VERSION and CHANGELOG.md are not as the release found them (a commit made during the review may ' +
            'hold them): check git log and git status') -ForegroundColor Red
        }
      }
    }
  }
}

# === 6. Commit, tag, push ===========================================================================================
Say ''
Say 'Committing, tagging, pushing'
if ($bump) {
  $undoFiles = @('git restore --staged --worktree -- VERSION CHANGELOG.md')
  Invoke-GitStep @('add', '--', 'VERSION', 'CHANGELOG.md') $undoFiles
  # The pathspec commits these two only, whatever else may have been staged during the prompts.
  Invoke-GitStep @('commit', '-m', "chore(release): $tag", '--', 'VERSION', 'CHANGELOG.md') $undoFiles
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
  Say "  2. It publishes the pre-release `"D2BS $tag`" (D2BS.dll, D2BS.pdb, the notes): $url/releases/tag/$tag"
  Say "  3. Give its D2BS.dll to the testers; the D2BS log and version() call it $($chosen -replace '-', 'U-')."
  Say '  4. Fixes for what they find add their CHANGELOG.md entries under [Unreleased]. Then run this again: for the'
  Say '     next release candidate, or for the final once a release candidate holds up.'
  Say '  If the build fails on a download, re-run it from its Actions page. If the code or the DLL is at fault and no'
  Say '  tester has the DLL yet:'
  Say "     gh release delete $tag --repo $repoArg --yes   (if the release exists)"
} else {
  Say "  2. Review the DRAFT release `"D2BS $tag`" (D2BS.dll, D2BS.pdb, the notes): $url/releases"
  Say '  3. Try the DLL on a test install, e.g. with one D2Bot profile.'
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
