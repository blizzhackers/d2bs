#Requires -Version 7.2
<#
  End-to-end test of build\Release-D2BS.ps1 and build\Get-ReleaseNotes.ps1 in a throwaway sandbox: a bare clone of
  this repo as "origin", and a working clone whose master is this checkout's HEAD plus the WORKING COPIES of the
  release scripts (an uncommitted change to them is what gets tested), with VERSION set to 1.6.4 and a CHANGELOG.md
  fixture. Each scenario runs the script with piped answers and checks the refs and files it leaves. Nothing reaches
  this repo or GitHub, and the sandbox is deleted afterwards (-Keep keeps it). Exit code = number of failed checks.

    pwsh -File build\tests\Test-ReleaseScript.ps1
#>
[CmdletBinding()]
param(
  [string]$Sandbox = (Join-Path ([IO.Path]::GetTempPath()) "d2bs-release-test-$PID"),
  [switch]$Keep
)
Set-StrictMode -Version 3
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Sandbox = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Sandbox)
$origin = Join-Path $Sandbox 'origin.git'
$work = Join-Path $Sandbox 'work'
$other = Join-Path $Sandbox 'other'
$script:failures = 0
# The files under test, copied from this checkout's build folder.
$scripts = @('Release-D2BS.ps1', 'Changelog.psm1', 'Get-ReleaseNotes.ps1')
$fixtureNotes = "### Added`n`n- **Sandbox feature.** One line.`n`n### Fixed`n`n- **Sandbox fix.** One line."
$fixture = "# Changelog`n`nIntro.`n`n## [Unreleased]`n`n$fixtureNotes`n`n[Unreleased]: https://github.com/blizzhackers/d2bs/compare/bf77a8f...HEAD`n"

function G([string]$Dir, [string[]]$GitArgs) {
  $out = & git -C $Dir @GitArgs 2>&1
  if ($LASTEXITCODE) { throw "git -C $Dir $($GitArgs -join ' ') failed: $out" }
  (@($out) -join "`n").Trim()
}
function Check([string]$Name, [bool]$Pass, [string]$Detail = '') {
  if ($Pass) { Write-Host "PASS $Name" -ForegroundColor Green }
  else { Write-Host "FAIL $Name $Detail" -ForegroundColor Red; $script:failures++ }
}
# Runs the release script in the sandbox with the given answers; returns its exit code and output.
function Invoke-Release([string[]]$Answers, [string[]]$ScriptArgs = @()) {
  $out = ($Answers -join "`n") | pwsh -NoProfile -File (Join-Path $work 'build' 'Release-D2BS.ps1') @ScriptArgs 2>&1
  [pscustomobject]@{ Code = $LASTEXITCODE; Text = (@($out) -join "`n") }
}
# Starts the release script with its input on a pipe, so a scenario can act while a prompt waits.
function Start-Release {
  $psi = [Diagnostics.ProcessStartInfo]::new('pwsh')
  foreach ($a in '-NoProfile', '-File', (Join-Path $work 'build' 'Release-D2BS.ps1')) { $psi.ArgumentList.Add($a) }
  $psi.RedirectStandardInput = $true
  $psi.RedirectStandardOutput = $true
  $psi.RedirectStandardError = $true
  $psi.UseShellExecute = $false
  $p = [Diagnostics.Process]::Start($psi)
  [pscustomobject]@{ Proc = $p; Out = [Text.StringBuilder]::new(); Buf = [char[]]::new(4096); Err = $p.StandardError.ReadToEndAsync(); Seen = 0 }
}
# Reads the script's output until it ends with the prompt (a prompt has no newline after it), asked after the last
# prompt waited for: the same prompt asked again needs new output.
function Wait-Prompt($Run, [string]$Prompt, [int]$TimeoutSec = 60) {
  $deadline = (Get-Date).AddSeconds($TimeoutSec)
  while (-not ($Run.Out.Length -gt $Run.Seen -and $Run.Out.ToString().EndsWith($Prompt))) {
    $read = $Run.Proc.StandardOutput.ReadAsync($Run.Buf, 0, $Run.Buf.Length)
    if (-not $read.Wait([int][Math]::Max(0, ($deadline - (Get-Date)).TotalMilliseconds))) { throw "no '$Prompt' within $TimeoutSec s: $($Run.Out)" }
    if ($read.Result -eq 0) { throw "the script ended before '$Prompt': $($Run.Out)" }
    [void]$Run.Out.Append($Run.Buf, 0, $read.Result)
  }
  $Run.Seen = $Run.Out.Length
}
function Send-Answer($Run, [string]$Answer) { $Run.Proc.StandardInput.WriteLine($Answer); $Run.Proc.StandardInput.Flush() }
function Complete-Release($Run) {
  $Run.Proc.StandardInput.Close()
  [void]$Run.Out.Append($Run.Proc.StandardOutput.ReadToEnd())
  if (-not $Run.Proc.WaitForExit(60000)) { throw 'the release script did not end' }
  [pscustomobject]@{ Code = $Run.Proc.ExitCode; Text = $Run.Out.ToString() + $Run.Err.Result }
}
function Invoke-Notes([string[]]$NotesArgs) {
  $out = & pwsh -NoProfile -File (Join-Path $work 'build' 'Get-ReleaseNotes.ps1') @NotesArgs 2>&1
  [pscustomobject]@{ Code = $LASTEXITCODE; Text = (@($out) -join "`n") }
}
# Get-ReleaseNotes as the build workflow runs it: the start-tag step output it sets, or $null when it fails or sets
# none.
function Get-StartTag([string]$Version) {
  $outputFile = Join-Path $Sandbox 'github-output.txt'
  [IO.File]::WriteAllText($outputFile, '')
  $saved = @($env:GITHUB_ACTIONS, $env:GITHUB_OUTPUT)
  $env:GITHUB_ACTIONS = 'true'
  $env:GITHUB_OUTPUT = $outputFile
  try { $n = Invoke-Notes @('-Version', $Version) } finally { $env:GITHUB_ACTIONS = $saved[0]; $env:GITHUB_OUTPUT = $saved[1] }
  $lines = @(Get-Content -LiteralPath $outputFile | Where-Object { $_ -like 'start-tag=*' })
  if ($n.Code -or $lines.Count -ne 1) { return $null }
  $lines[0].Substring('start-tag='.Length)
}
function Read-Changelog { (Get-Content -LiteralPath (Join-Path $work 'CHANGELOG.md') -Raw) -replace "`r`n", "`n" }
# Commits and pushes one entry right under [Unreleased], in its own group heading, so the next release has notes.
function Add-Entry([string]$Entry, [string]$Group = 'Fixed') {
  $text = Get-Content -LiteralPath (Join-Path $work 'CHANGELOG.md') -Raw
  $nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
  $text = $text.Replace("## [Unreleased]$nl", "## [Unreleased]$nl$nl### $Group$nl$nl$Entry$nl")
  [IO.File]::WriteAllText((Join-Path $work 'CHANGELOG.md'), $text)
  G $work @('commit', '--quiet', '-m', "sandbox: $Entry", '--', 'CHANGELOG.md') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
}
# The sandbox clones commit as their own identity (a CI runner has none).
function Set-Identity([string]$Dir) {
  G $Dir @('config', 'user.name', 'release-test') | Out-Null
  G $Dir @('config', 'user.email', 'release-test@example.invalid') | Out-Null
}

function Initialize-Sandbox {
  New-Item -ItemType Directory -Path $Sandbox | Out-Null
  $head = G $repo @('rev-parse', 'HEAD')
  & git clone --quiet --bare $repo $origin
  if ($LASTEXITCODE) { throw 'bare clone failed' }
  # The sandbox's master is this checkout's HEAD, whatever branch that is.
  G $origin @('update-ref', 'refs/heads/master', $head) | Out-Null
  G $origin @('symbolic-ref', 'HEAD', 'refs/heads/master') | Out-Null
  # The scenarios start before any release: the checkout's own tags (real releases, scratch tags) are dropped.
  foreach ($ref in @((G $origin @('for-each-ref', '--format=%(refname)', 'refs/tags')) -split "`n" | Where-Object { $_ })) {
    G $origin @('update-ref', '-d', $ref) | Out-Null
  }
  & git clone --quiet --branch master $origin $work
  if ($LASTEXITCODE) { throw 'clone failed' }
  # Its own .git, not a parent repo's (a path comparison would trip on 8.3 short names in TEMP).
  if ((G $work @('rev-parse', '--git-dir')) -ne '.git') { throw 'the sandbox clone is not its own repo' }
  if (G $work @('tag', '--list')) { throw 'the sandbox still has tags' }
  Set-Identity $work
  foreach ($file in $scripts) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot '..' $file) -Destination (Join-Path $work 'build') -Force }
  [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "1.6.4`n")
  # CRLF, as a Windows checkout has it: the release must keep the file's line endings.
  [IO.File]::WriteAllText((Join-Path $work 'CHANGELOG.md'), ($fixture -replace "`n", "`r`n"))
  G $work (@('add', '--') + @($scripts | ForEach-Object { "build/$_" }) + @('VERSION', 'CHANGELOG.md')) | Out-Null
  G $work @('commit', '--quiet', '--allow-empty', '-m', 'sandbox: the release script under test') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  "sandbox master: $(G $work @('log', '-1', '--format=%h %s')), VERSION $((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim())"
}

function Invoke-Scenarios {
  # 1. Dry run from a clean master: offers the release candidates first, then the finals; changes nothing.
  $head0 = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check 'dry run exits 0' ($r.Code -eq 0) $r.Text
  Check 'dry run offers the patch release candidate first' ($r.Text -match '1\) patch release candidate -> v1\.6\.5-rc\.1' -and
    $r.Text -match '2\) minor release candidate -> v1\.7\.0-rc\.1' -and $r.Text -match '3\) major release candidate -> v2\.0\.0-rc\.1') $r.Text
  Check 'then the finals' ($r.Text -match '4\) patch -> v1\.6\.5' -and $r.Text -match '5\) minor -> v1\.7\.0' -and $r.Text -match '6\) major -> v2\.0\.0') $r.Text
  Check 'dry run offers no "again" option' ($r.Text -notmatch 'again \(release commit') ''
  Check 'dry run creates no tag' (-not (G $work @('tag', '--list', 'v1.6.5'))) ''
  Check 'dry run leaves HEAD and VERSION' ((G $work @('rev-parse', 'HEAD')) -eq $head0 -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.4') ''
  Check 'dry run leaves CHANGELOG.md' (-not (G $work @('status', '--porcelain'))) ''
  Check 'dry run counts the Unreleased entries' ($r.Text -match 'CHANGELOG\.md \[Unreleased\]: Added 1, Fixed 1') $r.Text
  Check 'dry run names the minor version for Added entries' ($r.Text -match 'It has Added entries') ''
  Check 'dry run shows the release notes' ($r.Text -match 'Release notes \(CHANGELOG\.md \[Unreleased\]\)' -and $r.Text -match '    - \*\*Sandbox fix\.\*\* One line\.') $r.Text

  # 1b. With no release tag yet, a version that is not newer than VERSION needs a confirmation (1.6.4 shipped untagged).
  $r = Invoke-Release @('1', '1.6.4', 'n', '1.6.3', 'n')
  Check 'VERSION itself needs a confirmation' ($r.Text -match 'v1\.6\.4 is not newer than VERSION 1\.6\.4') $r.Text
  Check 'a lower version needs one too' ($r.Text -match 'v1\.6\.3 is not newer than VERSION 1\.6\.4') $r.Text
  Check 'declining both makes no tag' ($r.Code -eq 1 -and -not (G $work @('tag', '--list', 'v*'))) ''

  # 1c. Tagging VERSION's own number again needs that version's CHANGELOG.md section, which the build posts.
  $r = Invoke-Release @('1', '1.6.4', 'y', 'y')
  Check 'a tag without its section is refused' ($r.Code -eq 1 -and $r.Text -match 'no entries under \[1\.6\.4\]') $r.Text
  Check 'and makes no tag' (-not (G $work @('tag', '--list', 'v*'))) ''

  # 2. First release: a normal patch bump from the seeded 1.6.4.
  $today = Get-Date -Format 'yyyy-MM-dd'
  $r = Invoke-Release @('4', '', 'y', 'y')
  Check 'first release exits 0' ($r.Code -eq 0) $r.Text
  Check 'first release commits VERSION 1.6.5' ((G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.5' -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.5') ''
  Check 'v1.6.5 is annotated' ((G $work @('cat-file', '-t', 'v1.6.5')) -eq 'tag') ''
  Check 'v1.6.5 tags the release commit' ((G $work @('rev-parse', 'v1.6.5^{commit}')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin has v1.6.5' ([bool](G $origin @('tag', '--list', 'v1.6.5'))) ''
  Check 'tag message' ((G $work @('tag', '-l', '--format=%(contents:subject)', 'v1.6.5')) -eq 'D2BS v1.6.5') ''
  $log = Read-Changelog
  Check 'CHANGELOG.md dates the section' ($log.Contains("## [Unreleased]`n`n## [1.6.5] - $today`n`n$fixtureNotes`n")) $log
  Check 'CHANGELOG.md moves the links' ($log.EndsWith("[Unreleased]: https://github.com/blizzhackers/d2bs/compare/v1.6.5...HEAD`n[1.6.5]: https://github.com/blizzhackers/d2bs/compare/bf77a8f...v1.6.5`n")) $log
  Check 'CHANGELOG.md keeps CRLF' (-not ((Get-Content -LiteralPath (Join-Path $work 'CHANGELOG.md') -Raw) -match '(?<!\r)\n')) ''
  Check 'the release commit holds VERSION and CHANGELOG.md' ((@((G $work @('show', '--name-only', '--format=', 'HEAD')) -split "`n") -join ',') -eq 'CHANGELOG.md,VERSION') ''
  $n = Invoke-Notes @('-Version', 'v1.6.5')
  Check 'the release notes are the dated section' ($n.Code -eq 0 -and $n.Text -eq $fixtureNotes) $n.Text

  # 2b. No entries under [Unreleased] stops a new version; a dry run only warns. Group headings alone are no entries.
  $noEntries = '\[xx\] CHANGELOG\.md has no entries under \[Unreleased\]: the release would have no notes'
  $r = Invoke-Release @('4', '', 'y')
  Check 'an empty Unreleased stops the release' ($r.Code -eq 1 -and $r.Text -match $noEntries) $r.Text
  Check 'and the Context warned about it' ($r.Text -match 'a new version needs them there first') ''
  Check 'and leaves no tag and a clean tree' (-not (G $work @('tag', '--list', 'v1.6.6')) -and -not (G $work @('status', '--porcelain'))) ''
  $r = Invoke-Release @('1', '', 'y')
  Check 'an empty Unreleased stops a release candidate too' ($r.Code -eq 1 -and $r.Text -match $noEntries -and -not (G $work @('tag', '--list', 'v1.6.6-rc.1'))) $r.Text
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check 'a dry run only warns about it' ($r.Code -eq 0 -and $r.Text -match ($noEntries -replace '^\\\[xx\\\]', '\[!!\]')) $r.Text
  $file = Join-Path $work 'CHANGELOG.md'
  $text = Get-Content -LiteralPath $file -Raw
  $nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
  [IO.File]::WriteAllText($file, $text.Replace("## [Unreleased]$nl", "## [Unreleased]$nl$nl### Added$nl$nl### Fixed$nl"))
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check 'headings without entries are no entries' ($r.Code -eq 0 -and $r.Text -match ($noEntries -replace '^\\\[xx\\\]', '\[!!\]')) $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  Add-Entry '- **Second fix.** One line.'
  # Without the [Unreleased] link the release still dates the section, and says it adds no compare link.
  $text = Get-Content -LiteralPath $file -Raw
  [IO.File]::WriteAllText($file, ($text -replace '(?m)^\[Unreleased\]: .*\r?\n', ''))
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check 'a missing [Unreleased] link warns' ($r.Code -eq 0 -and $r.Text -match '\[!!\] CHANGELOG\.md has no \[Unreleased\] link at its end') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  Add-Content -LiteralPath $file -Value '<!-- a comment that never closes'
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check 'a comment that never closes warns' ($r.Code -eq 0 -and $r.Text -match '\[!!\] CHANGELOG\.md has an HTML comment or a code fence that never closes') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  Import-Module (Join-Path $work 'build' 'Changelog.psm1') -Force
  $dated = Update-ChangelogForRelease -Text "## [Unreleased]`n`n- x`n" -Version '9.0.0' -Date 'D' -Tag 'v9.0.0'
  Check 'and dates the section without adding a link' ($dated -eq "## [Unreleased]`n`n## [9.0.0] - D`n`n- x`n") $dated
  $dated = Update-ChangelogForRelease -Text "## [Unreleased]`n`n- x`n`n[unreleased]: https://h/compare/abc...HEAD`n" -Version '9.0.0' -Date 'D' -Tag 'v9.0.0'
  Check 'a lowercase [unreleased] link moves too' ($dated.EndsWith("[Unreleased]: https://h/compare/v9.0.0...HEAD`n[9.0.0]: https://h/compare/abc...v9.0.0`n")) $dated
  $counts = Get-ChangelogEntryCounts "### Fixed`n- a`n  - sub`n  - sub`n- b`n<!--`n- hidden"
  Check 'entries count top-level items only, and a comment left open hides the rest' ($counts.Count -eq 1 -and $counts['Fixed'] -eq 2) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')

  # 3. Patch release with the review gate's diff: VERSION and CHANGELOG.md commit, tag and push.
  $r = Invoke-Release @('4', '', 'y', 'd', 'y')
  Check 'patch release exits 0' ($r.Code -eq 0) $r.Text
  Check 'patch release offered 1.6.6' ($r.Text -match '4\) patch -> v1\.6\.6') ''
  Check 'the plan says the tag builds a draft' ($r.Text -match 'the tag builds a DRAFT release') ''
  Check 'VERSION is 1.6.6' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw) -eq "1.6.6`n") ''
  Check 'release commit subject' ((G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.6') ''
  Check 'release commit holds VERSION and CHANGELOG.md' ((@((G $work @('show', '--name-only', '--format=', 'HEAD')) -split "`n") -join ',') -eq 'CHANGELOG.md,VERSION') ''
  Check 'the 1.6.6 notes hold only the new entry' ((Invoke-Notes @('-Version', '1.6.6')).Text -eq "### Fixed`n`n- **Second fix.** One line.") ''
  Check 'the links chain from v1.6.5' ((Read-Changelog).EndsWith("[Unreleased]: https://github.com/blizzhackers/d2bs/compare/v1.6.6...HEAD`n[1.6.6]: https://github.com/blizzhackers/d2bs/compare/v1.6.5...v1.6.6`n[1.6.5]: https://github.com/blizzhackers/d2bs/compare/bf77a8f...v1.6.5`n")) (Read-Changelog)
  Check 'v1.6.6 tags the release commit' ((G $work @('rev-parse', 'v1.6.6^{commit}')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin master moved' ((G $origin @('rev-parse', 'master')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin has v1.6.6' ([bool](G $origin @('tag', '--list', 'v1.6.6'))) ''
  Check 'the d answer showed the diff' ($r.Text -match '\+1\.6\.6' -and $r.Text -match '\+## \[1\.6\.6\] - ') ''
  Add-Entry '- **Third fix.** One line.'

  # 4. A taken or malformed version is asked again; abort at review puts VERSION and CHANGELOG.md back.
  $r = Invoke-Release @('1', '1.6.6', '1.6.06', 'banana', '1.6.70000', '1.6.3', 'n', '1.7.0', 'y', 'n')
  Check 'abort at review exits 1' ($r.Code -eq 1) $r.Text
  Check 'existing tag refused' ($r.Text -match 'tag v1\.6\.6 already exists') ''
  Check 'a leading zero is the same version' (([regex]::Matches($r.Text, 'tag v1\.6\.6 already exists')).Count -eq 2) $r.Text
  Check 'malformed version refused' ($r.Text -match 'a version is major\.minor\.patch') ''
  Check 'a number over 65535 refused' (([regex]::Matches($r.Text, 'a version is major\.minor\.patch')).Count -eq 2) ''
  Check 'a version below the latest tag needs a confirmation' ($r.Text -match 'v1\.6\.3 is not newer than v1\.6\.6') ''
  Check 'VERSION put back' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.6') ''
  Check 'no tag v1.7.0' (-not (G $work @('tag', '--list', 'v1.7.0'))) ''
  Check 'tree clean after abort' (-not (G $work @('status', '--porcelain'))) ''
  Check '  and no copy of unedited files' ($r.Text -notmatch 'are kept in') $r.Text

  # 5. Preconditions: a tracked change stops a real run; a dry run only warns. So only a dry run gets past the edited
  #    VERSION's own tracked change to the VERSION check, which stops even a dry run.
  Add-Content -LiteralPath (Join-Path $work 'D2BS.h') -Value '// sandbox edit'
  $r = Invoke-Release @('1', '', 'y')
  Check 'tracked change stops the release' ($r.Code -eq 1 -and $r.Text -match 'tracked change') $r.Text
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check 'dry run only warns about it' ($r.Code -eq 0 -and $r.Text -match 'tracked change') $r.Text
  G $work @('checkout', '--quiet', '--', 'D2BS.h') | Out-Null
  [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "1.6.06`n")
  $r = Invoke-Release @() @('-DryRun')
  Check 'a VERSION with a leading zero is refused' ($r.Code -eq 1 -and $r.Text -match 'without leading zeros') $r.Text
  foreach ($bad in '1.6.6-rc.0', '1.6.6-rc.01', '1.6.6-RC.1', '1.6.6-beta.1') {
    [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "$bad`n")
    $r = Invoke-Release @() @('-DryRun')
    Check "a VERSION of $bad is refused" ($r.Code -eq 1 -and $r.Text -match "without leading zeros, not '$([regex]::Escape($bad))'") $r.Text
  }
  [IO.File]::WriteAllText((Join-Path $work 'VERSION'), '')
  $r = Invoke-Release @() @('-DryRun')
  Check 'an empty VERSION is refused by name' ($r.Code -eq 1 -and $r.Text -match "VERSION must hold major\.minor\.patch or major\.minor\.patch-rc\.N without leading zeros, not ''") $r.Text
  G $work @('checkout', '--quiet', '--', 'VERSION') | Out-Null
  [IO.File]::WriteAllText((Join-Path $work 'CHANGELOG.md'), '')
  $r = Invoke-Release @() @('-DryRun')
  Check 'an empty CHANGELOG.md is refused by name' ($r.Code -eq 1 -and $r.Text -match 'CHANGELOG\.md has no "## \[Unreleased\]" heading') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  # An untracked CHANGELOG.md: git could neither show it at the review nor put it back.
  G $work @('rm', '--quiet', '--cached', '--', 'CHANGELOG.md') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: untrack CHANGELOG.md') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'an untracked CHANGELOG.md stops the release' ($r.Code -eq 1 -and $r.Text -match 'CHANGELOG\.md is not committed') $r.Text
  G $work @('add', '--', 'CHANGELOG.md') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: track CHANGELOG.md again') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null

  # 6. Behind origin stops the release.
  & git clone --quiet --branch master $origin $other; if ($LASTEXITCODE) { throw 'clone failed' }
  Set-Identity $other
  Set-Content -LiteralPath (Join-Path $other 'sandbox.txt') -Value 'x'
  G $other @('add', 'sandbox.txt') | Out-Null
  G $other @('commit', '--quiet', '-m', 'sandbox: newer commit') | Out-Null
  G $other @('push', '--quiet', 'origin', 'master') | Out-Null
  $r = Invoke-Release @('1', '', 'y')
  Check 'behind origin stops the release' ($r.Code -eq 1 -and $r.Text -match 'behind origin/master') $r.Text
  G $work @('pull', '--quiet', 'origin', 'master') | Out-Null

  # 7. Another branch stops the release; running out of answers stops cleanly.
  G $work @('checkout', '--quiet', '-b', 'feature') | Out-Null
  $r = Invoke-Release @('1', '', 'y')
  Check 'wrong branch stops the release' ($r.Code -eq 1 -and $r.Text -match "not master") $r.Text
  G $work @('checkout', '--quiet', 'master') | Out-Null
  $r = Invoke-Release @('1')
  Check 'running out of answers stops cleanly' ($r.Code -eq 1 -and $r.Text -match 'no more input') $r.Text
  Check 'and leaves the tree clean' (-not (G $work @('status', '--porcelain'))) ''

  # 8. A mistyped flag is an error, never a real release.
  $tagsBefore = G $work @('tag', '--list', 'v*')
  foreach ($flag in '-DryRn', '--dry-run') {
    $r = Invoke-Release @('1', '', 'y', 'y') @($flag)
    Check "$flag is refused" ($r.Code -ne 0 -and $r.Text -match 'cannot be found') $r.Text
  }
  Check 'no tag from the mistyped flags' ((G $work @('tag', '--list', 'v*')) -eq $tagsBefore) ''

  # 9. Input that ends at the review prompt puts VERSION and CHANGELOG.md back.
  $r = Invoke-Release @('1', '', 'y')
  Check 'end of input at review stops' ($r.Code -eq 1 -and $r.Text -match 'no more input') $r.Text
  Check 'and puts VERSION and CHANGELOG.md back' ($r.Text -match 'VERSION put back' -and -not (G $work @('status', '--porcelain'))) $r.Text

  # 10. A rejected push prints undo commands that work as printed.
  $hook = Join-Path $origin 'hooks' 'pre-receive'
  [IO.File]::WriteAllText($hook, "#!/bin/sh`necho sandbox rejects pushes`nexit 1`n")
  $head = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('4', '', 'y', 'y')
  Check 'rejected push exits 1' ($r.Code -eq 1 -and $r.Text -match 'push failed') $r.Text
  $undo = @($r.Text -split "`n" | Where-Object { $_ -match '^  git ' } | ForEach-Object { $_.Trim() })
  Check 'undo commands are separate lines' ($undo.Count -eq 3 -and $undo[0] -eq 'git reset --keep HEAD~1' -and $undo[1] -eq 'git tag -d v1.6.7') ($undo -join ' | ')
  $shown = if ($r.Text -match 'The release commit keeps the CHANGELOG\.md you reviewed: git show ([0-9a-f]+):CHANGELOG\.md') { $Matches[1] } else { '' }
  Check '  and names the release commit that keeps the reviewed CHANGELOG.md' ($shown -and $shown -eq (G $work @('rev-parse', '--short', 'HEAD'))) $r.Text
  Check 'origin got nothing' (-not (G $origin @('tag', '--list', 'v1.6.7')) -and (G $origin @('rev-parse', 'master')) -eq $head) ''
  # An edit to another tracked file, as if made during the prompts, must survive the printed undo.
  Add-Content -LiteralPath (Join-Path $work 'D2BS.h') -Value '// edit made during the prompts'
  foreach ($u in $undo | Select-Object -First 2) { G $work @($u.Substring(4) -split ' ') | Out-Null }
  Check 'the undo restores the release state' ((G $work @('rev-parse', 'HEAD')) -eq $head -and -not (G $work @('tag', '--list', 'v1.6.7')) -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.6') ''
  Check 'and CHANGELOG.md' (-not (G $work @('status', '--porcelain', '--', 'CHANGELOG.md')) -and (Read-Changelog).Contains("## [Unreleased]`n`n### Fixed`n`n- **Third fix.**")) ''
  Check 'and keeps the other edit' ((Get-Content -LiteralPath (Join-Path $work 'D2BS.h') -Raw) -match 'edit made during the prompts') ''
  G $work @('checkout', '--quiet', '--', 'D2BS.h') | Out-Null
  Remove-Item -LiteralPath $hook

  # 11. After a failed tag build: delete the tag, fix master, and release the same version again (tag only).
  G $work @('push', '--quiet', 'origin', ':refs/tags/v1.6.6') | Out-Null
  G $work @('tag', '-d', 'v1.6.6') | Out-Null
  Set-Content -LiteralPath (Join-Path $work 'fix.txt') -Value 'the fix'
  G $work @('add', 'fix.txt') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: the fix') | Out-Null
  $fixHead = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('y', '1', '', 'y')
  Check 're-release exits 0' ($r.Code -eq 0) $r.Text
  Check 'it offered 1.6.6 again first' ($r.Text -match '1\) 1\.6\.6 again \(release commit') ''
  Check 're-release adds no commit' ((G $work @('rev-parse', 'HEAD')) -eq $fixHead) ''
  Check 'v1.6.6 now tags the fixed master' ((G $work @('rev-parse', 'v1.6.6^{commit}')) -eq $fixHead -and (G $origin @('rev-parse', 'v1.6.6^{commit}')) -eq $fixHead) ''
  Check 'origin master has the fix' ((G $origin @('rev-parse', 'master')) -eq $fixHead) ''
  Check 'it showed the 1.6.6 notes' ($r.Text -match 'Release notes \(CHANGELOG\.md \[1\.6\.6\]\)' -and $r.Text -match '    - \*\*Second fix\.\*\*') $r.Text
  Check 'and warned that [Unreleased] entries stay out' ($r.Text -match '\[!!\] \[Unreleased\] has entries, and re-releasing v1\.6\.6 posts only \[1\.6\.6\]') $r.Text

  # 12. Get-ReleaseNotes: the file the release job posts, and the failures that stop a tag build.
  $notesFile = Join-Path $Sandbox 'out' 'release-notes.md'
  $n = Invoke-Notes @('-Version', 'v1.6.5', '-OutFile', $notesFile)
  $bytes = [IO.File]::ReadAllBytes($notesFile)
  Check 'notes file: the section, one newline, no BOM' ($n.Code -eq 0 -and [Text.Encoding]::UTF8.GetString($bytes) -eq "$fixtureNotes`n" -and $bytes[0] -ne 0xEF) $n.Text
  $n = Invoke-Notes @('-Version', '9.9.9')
  Check 'a missing section fails' ($n.Code -eq 1 -and $n.Text -match 'no \[9\.9\.9\] section') $n.Text
  $odd = Join-Path $Sandbox 'odd.md'
  $fence = '```'
  [IO.File]::WriteAllText($odd, ("## [Unreleased]`n`n## [5.0.0] - 2026-04-01`n`n<!--`n- **Lead.** A template.`n-->`n`n* * *`n`n" +
      "$fence`n- in a fence`n$fence`n`n## [3.0.0] - 2026-02-01`n`n### Added`n`n## [2.0.0] - 2026-01-01`n`n" +
      "## [1.0.0] - 2025-01-01`n`n- **Wrapped.** A line that goes`n  on here.`n  - a nested item`n[1]: https://example.invalid`n" +
      "- **After a link.** Still in the section.`n`n[unreleased]: https://example.invalid/compare/v3.0.0...HEAD`n"))
  $n = Invoke-Notes @('-Version', '5.0.0', '-Changelog', $odd)
  Check 'list lines in a comment or a fence, and a thematic break, are no entries' ($n.Code -eq 1 -and $n.Text -match '\[5\.0\.0\] section has no entries') $n.Text
  $n = Invoke-Notes @('-Version', '2.0.0', '-Changelog', $odd)
  Check 'an empty section fails' ($n.Code -eq 1 -and $n.Text -match '\[2\.0\.0\] section has no entries') $n.Text
  $n = Invoke-Notes @('-Version', '3.0.0', '-Changelog', $odd)
  Check 'a section of headings only fails' ($n.Code -eq 1 -and $n.Text -match '\[3\.0\.0\] section has no entries') $n.Text
  $n = Invoke-Notes @('-Version', '1.0.0', '-Changelog', $odd)
  Check 'a wrapped line warns once, a nested item not' ($n.Code -eq 0 -and ([regex]::Matches($n.Text, 'a wrapped line')).Count -eq 1 -and $n.Text -match 'join it to its entry: on here\.') $n.Text
  Check 'a link inside a section does not end it; the version links do, in any case' ($n.Text -match 'After a link' -and $n.Text -notmatch 'example\.invalid/compare') $n.Text
  $one = Join-Path $Sandbox 'one-line.md'
  [IO.File]::WriteAllText($one, '## [4.0.0] - 2026-03-01')
  $n = Invoke-Notes @('-Version', '4.0.0', '-Changelog', $one)
  Check 'a one-line file without a newline reads' ($n.Code -eq 1 -and $n.Text -match '\[4\.0\.0\] section has no entries') $n.Text
  $empty = Join-Path $Sandbox 'empty.md'
  [IO.File]::WriteAllText($empty, '')
  $n = Invoke-Notes @('-Version', '4.0.0', '-Changelog', $empty)
  Check 'an empty file has no section' ($n.Code -eq 1 -and $n.Text -match 'no \[4\.0\.0\] section') $n.Text

  # 13. A commit made while a prompt waits stops the release before anything is published: at "Proceed?" before
  #     anything is written, and at the review, which puts the files back.
  foreach ($at in 'Proceed? [Y/n] ', '(y) commit + tag + push  (d) show the diff  (n) abort: ') {
    $where = if ($at -like 'Proceed*') { 'Proceed?' } else { 'the review' }
    $originHead = G $origin @('rev-parse', 'master')
    $run = Start-Release
    try {
      Wait-Prompt $run 'Choice [1]: '
      Send-Answer $run '4'
      Wait-Prompt $run 'Version [1.6.7]: '
      Send-Answer $run ''
      if ($where -eq 'the review') { Wait-Prompt $run 'Proceed? [Y/n] '; Send-Answer $run 'y' }
      Wait-Prompt $run $at
      Set-Content -LiteralPath (Join-Path $work 'unseen.txt') -Value 'x'
      G $work @('add', '--', 'unseen.txt') | Out-Null
      G $work @('commit', '--quiet', '-m', 'sandbox: a commit made while a prompt waited') | Out-Null
      Send-Answer $run 'y'
      $r = Complete-Release $run
    } finally {
      if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
    }
    Check "a commit made at $where stops the release" ($r.Code -eq 1 -and $r.Text -match 'HEAD changed during the') $r.Text
    Check "  and publishes nothing ($where)" (-not (G $work @('tag', '--list', 'v1.6.7')) -and (G $origin @('rev-parse', 'master')) -eq $originHead) ''
    Check "  and leaves VERSION and CHANGELOG.md as committed ($where)" ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.6' -and -not (G $work @('status', '--porcelain'))) (G $work @('status', '--porcelain'))
    G $work @('reset', '--quiet', '--keep', 'HEAD~1') | Out-Null
  }

  # 13b. A commit made at the review that takes the release files with it (git commit -a) keeps them: the script
  #      must not claim it put them back.
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '4'
    Wait-Prompt $run 'Version [1.6.7]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run '(y) commit + tag + push  (d) show the diff  (n) abort: '
    G $work @('commit', '--quiet', '-am', 'sandbox: a commit that takes the release files') | Out-Null
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'a commit that takes the release files stops the release' ($r.Code -eq 1 -and $r.Text -match 'HEAD changed during the review') $r.Text
  Check '  and says the files are not as it found them' ($r.Text -match 'not as the release found them' -and $r.Text -notmatch 'VERSION put back') $r.Text
  G $work @('reset', '--quiet', '--keep', 'HEAD~1') | Out-Null
  if ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -ne '1.6.6' -or (G $work @('status', '--porcelain'))) { throw 'the sandbox did not return to 1.6.6' }

  # 13c. An edit to CHANGELOG.md made while "Proceed?" waits stops the release, and the edit survives.
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '4'
    Wait-Prompt $run 'Version [1.6.7]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Add-Content -LiteralPath (Join-Path $work 'CHANGELOG.md') -Value '- **Edit made at Proceed.** One line.'
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'an edit made at Proceed? stops the release' ($r.Code -eq 1 -and $r.Text -match 'changed during the prompts') $r.Text
  Check '  and keeps the edit' ((Read-Changelog).Contains('Edit made at Proceed.') -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.6') ''
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null

  # 14. The first release candidate dates [Unreleased] as its own section and chains its link from the last final.
  $link = 'https://github.com/blizzhackers/d2bs/compare'
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'the first release candidate exits 0' ($r.Code -eq 0) $r.Text
  Check 'VERSION is 1.6.7-rc.1' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw) -eq "1.6.7-rc.1`n") ''
  Check 'the commit and the annotated tag name the candidate' ((G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.7-rc.1' -and
    (G $work @('cat-file', '-t', 'v1.6.7-rc.1')) -eq 'tag' -and [bool](G $origin @('tag', '--list', 'v1.6.7-rc.1'))) ''
  Check 'the plan says the tag publishes a pre-release' ($r.Text -match 'the tag builds and publishes a pre-release') $r.Text
  Check 'the next steps hand the DLL to testers' ($r.Text -match 'call it 1\.6\.7U-rc\.1\.' -and $r.Text -match 'cut the next release candidate instead' -and
    $r.Text -notmatch 'draft=false') $r.Text
  $log = Read-Changelog
  Check 'CHANGELOG.md dates [Unreleased] as the candidate' ($log.Contains("## [Unreleased]`n`n## [1.6.7-rc.1] - $today`n`n### Fixed`n`n- **Third fix.** One line.`n`n## [1.6.6]")) $log
  Check 'and chains its link from v1.6.6' ($log.Contains("[Unreleased]: $link/v1.6.7-rc.1...HEAD`n[1.6.7-rc.1]: $link/v1.6.6...v1.6.7-rc.1`n[1.6.6]: ")) $log
  Check 'its notes are its section' ((Invoke-Notes @('-Version', 'v1.6.7-rc.1')).Text -eq "### Fixed`n`n- **Third fix.** One line.") ''
  Check 'its pull request list starts at v1.6.6' ((Get-StartTag 'v1.6.7-rc.1') -eq 'v1.6.6') ''
  Check 'a link that starts from a commit gives no start tag' ((Get-StartTag 'v1.6.5') -eq '') ''

  # 14b. After a failed build, before any tester has it, the same candidate again tags the fixed master.
  G $work @('push', '--quiet', 'origin', ':refs/tags/v1.6.7-rc.1') | Out-Null
  G $work @('tag', '-d', 'v1.6.7-rc.1') | Out-Null
  Set-Content -LiteralPath (Join-Path $work 'fix2.txt') -Value 'the candidate fix'
  G $work @('add', 'fix2.txt') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: the candidate fix') | Out-Null
  $fixHead = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('y', '1', '', 'y')
  Check 'the candidate again exits 0' ($r.Code -eq 0) $r.Text
  Check 'it offered 1.6.7-rc.1 again first, then the next candidate and the final' ($r.Text -match '1\) 1\.6\.7-rc\.1 again \(release commit' -and
    $r.Text -match '2\) next release candidate -> v1\.6\.7-rc\.2' -and $r.Text -match '3\) final -> v1\.6\.7') $r.Text
  Check 'v1.6.7-rc.1 tags the fixed master, with no new commit' ((G $work @('rev-parse', 'HEAD')) -eq $fixHead -and
    (G $origin @('rev-parse', 'v1.6.7-rc.1^{commit}')) -eq $fixHead) ''
  Check 'it warned that the [Unreleased] link starts at the deleted tag' ($r.Text -match "\[!!\] CHANGELOG\.md's \[Unreleased\] link starts at v1\.6\.7-rc\.1, which is no local tag: if its build failed, release 1\.6\.7-rc\.1 again first") $r.Text

  # 14c. The next candidate holds only what is new since the one before.
  Add-Entry '- **Candidate feature.** One line.' 'Added'
  Add-Entry '- **Candidate-only fix.** One line.'
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'the second release candidate exits 0' ($r.Code -eq 0) $r.Text
  Check 'it counted the first candidate and [Unreleased]' ($r.Text -match 'CHANGELOG\.md \[1\.6\.7-rc\.1\]: Fixed 1' -and $r.Text -match 'CHANGELOG\.md \[Unreleased\]: Fixed 1, Added 1') $r.Text
  Check 'it showed the commits since v1.6.7-rc.1' ($r.Text -match '2 commit\(s\) since v1\.6\.7-rc\.1:') $r.Text
  Check 'VERSION is 1.6.7-rc.2' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw) -eq "1.6.7-rc.2`n") ''
  Check 'its notes hold only the new entries' ((Invoke-Notes @('-Version', '1.6.7-rc.2')).Text -eq
    "### Fixed`n`n- **Candidate-only fix.** One line.`n`n### Added`n`n- **Candidate feature.** One line.") ''
  Check 'its pull request list starts at v1.6.7-rc.1' ((Get-StartTag 'v1.6.7-rc.2') -eq 'v1.6.7-rc.1') ''

  # 14d. Inside a cycle: taken, malformed and older versions are asked again, and another candidate needs new entries.
  $r = Invoke-Release @('1', '1.6.7-rc.2', '1.6.7-rc.02', '1.6.7-beta.1', '1.6.7-rc.0', '1.6.6-rc.9', 'n', '')
  Check 'a candidate with nothing new stops' ($r.Code -eq 1 -and $r.Text -match $noEntries) $r.Text
  Check '  and the Context said a final can still merge them' ($r.Text -match 'a final can still merge the release candidates') $r.Text
  Check '  taken candidates refused, a leading zero included' (([regex]::Matches($r.Text, 'tag v1\.6\.7-rc\.2 already exists')).Count -eq 2) $r.Text
  Check '  other labels and rc.0 refused' (([regex]::Matches($r.Text, 'a version is major\.minor\.patch, or')).Count -eq 2) $r.Text
  Check '  an older candidate needs a confirmation' ($r.Text -match 'v1\.6\.6-rc\.9 is not newer than v1\.6\.7-rc\.2') $r.Text
  Check '  the next candidate was rc.3, and nothing was tagged' ($r.Text -match '1\) next release candidate -> v1\.6\.7-rc\.3' -and
    -not (G $work @('tag', '--list', 'v1.6.7-rc.3')) -and -not (G $work @('status', '--porcelain'))) ''
  Check '  with nothing new, the final was the default' ($r.Text -match 'Choice \[2\]: ') $r.Text

  # 14e. Leaving the final's review keeps the edits made there: a section left without entries is asked about again,
  #      and n copies the edited CHANGELOG.md out of the repo before it puts the files back.
  Add-Entry '- **Change after the last candidate.** One line.' 'Changed'
  $review = '(y) commit + tag + push  (d) show the diff  (n) abort: '
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '2'
    Wait-Prompt $run 'Version [1.6.7]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    $file = Join-Path $work 'CHANGELOG.md'
    [IO.File]::WriteAllText($file, ((Get-Content -LiteralPath $file -Raw) -replace '(?m)^## \[1\.6\.7\] - .*\r?\n', ''))
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'a review edit that leaves no entries is asked about again' ($r.Code -eq 1 -and $r.Text -match 'CHANGELOG\.md has no entries under \[1\.6\.7\] now') $r.Text
  $kept = if ($r.Text -match '(?m)Your edits to CHANGELOG\.md are kept in (.+?)\r?$') { $Matches[1] } else { $null }
  $keptText = if ($kept -and (Test-Path -LiteralPath $kept)) { Get-Content -LiteralPath $kept -Raw } else { '' }
  Check '  and n keeps the edited CHANGELOG.md outside the repo' ($keptText -match 'Candidate feature' -and $keptText -notmatch '## \[1\.6\.7\] - ') $r.Text
  Check '  and puts the files back' ($r.Text -match 'VERSION put back to 1\.6\.7-rc\.2' -and -not (G $work @('status', '--porcelain'))) (G $work @('status', '--porcelain'))
  if ($keptText) { Remove-Item -LiteralPath $kept }

  # 14f. The final merges the candidates and [Unreleased]; the review takes an edit to the merged notes, and asks again
  #      while VERSION no longer names the release.
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '2'
    Wait-Prompt $run 'Version [1.6.7]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "9.9.9`n")
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "1.6.7`n")
    $file = Join-Path $work 'CHANGELOG.md'
    [IO.File]::WriteAllText($file, ((Get-Content -LiteralPath $file -Raw) -replace '- \*\*Candidate-only fix\.\*\* One line\.\r?\n', ''))
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'the final exits 0' ($r.Code -eq 0) $r.Text
  Check 'it offered the next candidate first, then the final' ($r.Text -match '1\) next release candidate -> v1\.6\.7-rc\.3' -and $r.Text -match '2\) final -> v1\.6\.7') $r.Text
  Check 'it warned about the commit no candidate had' ($r.Text -match 'v1\.6\.7 would ship 1 commit\(s\) that no release candidate had \(since v1\.6\.7-rc\.2\)') $r.Text
  Check 'the plan merges both candidates and [Unreleased]' ($r.Text -match "merge CHANGELOG\.md's \[1\.6\.7-rc\.2\], \[1\.6\.7-rc\.1\] and \[Unreleased\] sections into \[1\.6\.7\]" -and
    $r.Text -match 'Release notes \(CHANGELOG\.md \[Unreleased\] and \[1\.6\.7-rc\.2\], \[1\.6\.7-rc\.1\], merged\)') $r.Text
  Check 'the review asked again while VERSION said 9.9.9' ($r.Text -match 'VERSION no longer says 1\.6\.7') $r.Text
  Check 'VERSION is 1.6.7, and origin has v1.6.7 on the release commit' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw) -eq "1.6.7`n" -and
    (G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.7' -and (G $origin @('rev-parse', 'v1.6.7^{commit}')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  $log = Read-Changelog
  $merged = "## [Unreleased]`n`n## [1.6.7] - $today`n`n### Added`n`n- **Candidate feature.** One line.`n`n### Changed`n`n" +
    "- **Change after the last candidate.** One line.`n`n### Fixed`n`n- **Third fix.** One line.`n`n## [1.6.6]"
  Check 'the final merged the candidates in group order, without the edited-out entry' ($log.Contains($merged)) $log
  Check 'no candidate section or link is left' ($log -notmatch '1\.6\.7-rc\.') $log
  Check 'its link starts where the first candidate did' ($log.Contains("[Unreleased]: $link/v1.6.7...HEAD`n[1.6.7]: $link/v1.6.6...v1.6.7`n[1.6.6]: ")) $log
  Check 'CHANGELOG.md still has CRLF only' (-not ((Get-Content -LiteralPath (Join-Path $work 'CHANGELOG.md') -Raw) -match '(?<!\r)\n')) ''
  Check 'its notes are the merged section' ((Invoke-Notes @('-Version', '1.6.7')).Text -eq ($merged -replace '(?s)^.*?- \d{4}-\d\d-\d\d\n\n', '' -replace '\n\n## \[1\.6\.6\]$', '')) ''
  Check 'its pull request list starts at v1.6.6' ((Get-StartTag 'v1.6.7') -eq 'v1.6.6') ''
  Check 'the next steps are a final''s' ($r.Text -match 'draft=false --latest' -and $r.Text -notmatch 'call it') $r.Text

  # 14g. The final again after a failed build: its own release commit is not one that no candidate had.
  G $work @('push', '--quiet', 'origin', ':refs/tags/v1.6.7') | Out-Null
  G $work @('tag', '-d', 'v1.6.7') | Out-Null
  $r = Invoke-Release @('1', '', 'y')
  Check 'the final again exits 0' ($r.Code -eq 0 -and $r.Text -match '1\) 1\.6\.7 again \(release commit') $r.Text
  Check '  and counts only the commit after the candidate' ($r.Text -match 'v1\.6\.7 would ship 1 commit\(s\) that no release candidate had') $r.Text
  Check '  and offers the next cycle after it' ($r.Text -match '2\) patch release candidate -> v1\.6\.8-rc\.1' -and $r.Text -match '5\) patch -> v1\.6\.8') $r.Text

  # 14h. The merge on its own: text before any group, a group in another case, an unknown group and an empty one, a
  #      final version below the candidates, no Unreleased link, hidden text and a paragraph after a list.
  Import-Module (Join-Path $work 'build' 'Changelog.psm1') -Force
  $in = ("# Changelog`n`n## [Unreleased]`n`n### fixed`n`n- u`n`n## [2.0.0-rc.2] - D2`n`n### Fixed`n`n- c`n`n### Notes`n`n- n`n`n" +
    "## [2.0.0-rc.1] - D1`n`n- loose`n`n### Security`n`n- s`n`n### Fixed`n`n- a`n  - nested`n`n### Added`n`n## [1.0.0] - D0`n`n- old`n`n" +
    "[Unreleased]: https://h/compare/v2.0.0-rc.2...HEAD`n[2.0.0-rc.2]: https://h/compare/v2.0.0-rc.1...v2.0.0-rc.2`n" +
    "[2.0.0-rc.1]: https://h/compare/v1.0.0...v2.0.0-rc.1`n[1.0.0]: https://h/compare/abc...v1.0.0`n")
  $out = Update-ChangelogForRelease -Text $in -Version '2.0.0' -Date 'D' -Tag 'v2.0.0'
  $want = ("# Changelog`n`n## [Unreleased]`n`n## [2.0.0] - D`n`n- loose`n`n### Fixed`n`n- a`n  - nested`n- c`n- u`n`n### Security`n`n- s`n`n" +
    "### Notes`n`n- n`n`n## [1.0.0] - D0`n`n- old`n`n[Unreleased]: https://h/compare/v2.0.0...HEAD`n[2.0.0]: https://h/compare/v1.0.0...v2.0.0`n" +
    "[1.0.0]: https://h/compare/abc...v1.0.0`n")
  Check 'the merge orders groups and keeps older versions' ($out -eq $want) $out
  $out = Update-ChangelogForRelease -Text "## [Unreleased]`n`n## [3.0.0-rc.1] - D`n`n- x" -Version '3.0.0' -Date 'E' -Tag 'v3.0.0'
  Check 'a lone candidate without links merges, ending as it did' ($out -eq "## [Unreleased]`n`n## [3.0.0] - E`n`n- x") $out
  $threw = $null
  try { Update-ChangelogForRelease -Text "## [Unreleased]`n`n## [3.0.0-rc.1] - D`n`n### Added`n" -Version '3.0.0' -Date 'E' -Tag 'v3.0.0' | Out-Null }
  catch { $threw = $_.Exception.Message }
  Check 'a final with no entries anywhere is refused' ($threw -match 'no entries under \[Unreleased\] or its release candidates') $threw
  $out = Update-ChangelogForRelease -Text $in -Version '2.0.0-rc.3' -Date 'D' -Tag 'v2.0.0-rc.3'
  Check 'a release candidate leaves the other candidates alone' ($out.Contains("## [Unreleased]`n`n## [2.0.0-rc.3] - D`n`n### fixed`n`n- u`n`n## [2.0.0-rc.2] - D2") -and
    $out.Contains("[Unreleased]: https://h/compare/v2.0.0-rc.3...HEAD`n[2.0.0-rc.3]: https://h/compare/v2.0.0-rc.2...v2.0.0-rc.3`n[2.0.0-rc.2]: ")) $out
  # Hidden text (an HTML comment block, a fenced block closed only by a fence at least as long) holds no heading, link
  # or entry for any reader; a "<!--" later in a line opens nothing.
  $fence3 = '```'
  $fence4 = '````'
  $in = ("## [Unreleased]`n`n<!--`n### Added`n`n- **Lead.** A template.`n-->`n`n## [2.0.0-rc.1] - D`n`n### Added`n`n" +
    "- **Real feature.** One line.`n`n$fence4`n$fence3`n### Fixed`n$fence4`n")
  $section = Get-ChangelogSection -Text (Update-ChangelogForRelease -Text $in -Version '2.0.0' -Date 'E' -Tag 'v2.0.0') -Name '2.0.0'
  Check 'a commented template and a fenced heading stay whole, and the entries visible' ($section -eq
    "<!--`n### Added`n`n- **Lead.** A template.`n-->`n`n### Added`n`n- **Real feature.** One line.`n`n$fence4`n$fence3`n### Fixed`n$fence4" -and
    (Get-ChangelogEntryCounts $section)['Added'] -eq 1) $section
  $in = ("## [Unreleased]`n`n### Fixed`n`n- u`n`n<!--`n## [x.y.z] - yyyy-mm-dd`n### Added`n-->`n`n## [2.0.0-rc.1] - D`n`n### Added`n`n" +
    "- **Real feature.** One line.`n`n[Unreleased]: https://h/compare/v2.0.0-rc.1...HEAD`n[2.0.0-rc.1]: https://h/compare/v1.0.0...v2.0.0-rc.1`n")
  Check 'a commented-out section heading hides no release candidate' ((Get-ChangelogCandidates $in).Count -eq 1) ''
  $out = Update-ChangelogForRelease -Text $in -Version '2.0.0' -Date 'E' -Tag 'v2.0.0'
  Check '  and the final merges it, its link from v1.0.0' ($out -notmatch '2\.0\.0-rc\.1' -and $out.Contains('[2.0.0]: https://h/compare/v1.0.0...v2.0.0') -and
    (Get-ChangelogEntryCounts (Get-ChangelogSection -Text $out -Name '2.0.0'))['Added'] -eq 1) $out
  $section = Get-ChangelogSection -Text (Update-ChangelogForRelease -Text "## [Unreleased]`n`n## [2.0.0-rc.1] - D`n`n### Added`n`n- a <!-- todo`n`n### Fixed`n`n- c`n" -Version '2.0.0' -Date 'E' -Tag 'v2.0.0') -Name '2.0.0'
  Check 'an unclosed <!-- inside an entry opens no hidden block' ($section -eq "### Added`n`n- a <!-- todo`n`n### Fixed`n`n- c") $section
  $counts = Get-ChangelogEntryCounts "$fence3`n<!--`n$fence3`n### Fixed`n- a"
  Check 'a <!-- inside a fence hides nothing after the fence' ($counts.Count -eq 1 -and $counts['Fixed'] -eq 1) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  Check 'a commented-out link is not the link' ((Get-ChangelogCompareBase "<!--`n[Unreleased]: https://h/compare/old...HEAD`n-->`n[Unreleased]: https://h/compare/v1.0.0...HEAD" 'Unreleased') -eq 'v1.0.0') ''
  $section = Get-ChangelogSection -Text (Update-ChangelogForRelease -Text "## [Unreleased]`n`nPara two.`n`n## [2.0.0-rc.1] - D`n`n- loose one`n" -Version '2.0.0' -Date 'E' -Tag 'v2.0.0') -Name '2.0.0'
  Check 'a paragraph after a list stays its own paragraph' ($section -eq "- loose one`n`nPara two.") $section
  $section = Get-ChangelogSection -Text (Update-ChangelogForRelease -Text "## [Unreleased]`n`n2. two`n`n## [2.0.0-rc.1] - D`n`nPara one.`n" -Version '2.0.0' -Date 'E' -Tag 'v2.0.0') -Name '2.0.0'
  Check 'a list that cannot interrupt a paragraph starts after a blank line' ($section -eq "Para one.`n`n2. two") $section
  # The same with CRLF: a "~~~" fence and a one-line comment hide a heading and links, "<!-->" closes on its own line,
  # and a line of backticks with a backtick after them is inline code, not a fence.
  $in = ("## [Unreleased]`n`n- e`n`n<!-->`n<!-- [Unreleased]: https://h/compare/bad...HEAD -->`n${fence3}x${fence3}`n`n" +
    "## [1.0.0-rc.1] - D`n`n### Fixed`n`n- r`n`n~~~`n## [9.9.9]`n### Added`n[1.0.0-rc.1]: https://h/compare/bad...v1.0.0-rc.1`n~~~`n`n" +
    "[Unreleased]: https://h/compare/v1.0.0-rc.1...HEAD`n[1.0.0-rc.1]: https://h/compare/v0.9.0...v1.0.0-rc.1`n") -replace "`n", "`r`n"
  Check 'CRLF hidden text: one candidate, the visible link, nothing left open' ((Get-ChangelogCandidates $in).Count -eq 1 -and
    (Get-ChangelogCompareBase $in 'Unreleased') -eq 'v1.0.0-rc.1' -and -not (Test-ChangelogUnclosed $in)) ''
  $out = Update-ChangelogForRelease -Text $in -Version '1.0.0' -Date 'E' -Tag 'v1.0.0'
  $counts = Get-ChangelogEntryCounts (Get-ChangelogSection -Text $out -Name '1.0.0')
  Check '  and the final merges it with its fence whole, linked from v0.9.0' ($out.Contains("~~~`r`n## [9.9.9]`r`n### Added`r`n[1.0.0-rc.1]: https://h/compare/bad...v1.0.0-rc.1`r`n~~~") -and
    $out.Contains('[1.0.0]: https://h/compare/v0.9.0...v1.0.0') -and $counts['Fixed'] -eq 1 -and $counts['(no group)'] -eq 1 -and
    $out -cnotmatch '(?m)^## \[1\.0\.0-rc\.1\]') $out
}

if (Test-Path -LiteralPath $Sandbox) { throw "$Sandbox already exists: pass another -Sandbox" }
try {
  Initialize-Sandbox
  Invoke-Scenarios
} finally {
  if ($Keep) { "sandbox kept: $Sandbox" }
  else { Remove-Item -LiteralPath $Sandbox -Recurse -Force -ErrorAction SilentlyContinue }
}
"failures: $script:failures"
exit $script:failures
