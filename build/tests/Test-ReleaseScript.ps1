#Requires -Version 7.2
<#
  End-to-end test of build\Release-D2BS.ps1 and build\Get-ReleaseNotes.ps1 in a throwaway sandbox: a bare clone of
  this repo as "origin", and a working clone whose master is this checkout's HEAD plus the WORKING COPIES of the
  release scripts and changelog.d\README.md (an uncommitted change to them is what gets tested) and no other
  changelog.d file, with VERSION set to 1.6.4 and a CHANGELOG.md fixture. Each scenario runs the script with piped
  answers and checks the refs and files it leaves. Nothing reaches this repo or GitHub, and the sandbox is deleted
  afterwards (-Keep keeps it). Exit code = number of failed checks.

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
$fragmentDir = Join-Path $work 'changelog.d'
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
# changelog.d files: written as given (text as UTF-8 without a BOM, line endings included; a byte array as it is),
# in a folder when the name has one, then committed and pushed together unless -NoCommit.
function Add-Fragments([System.Collections.Specialized.OrderedDictionary]$Files, [string]$Message = 'changelog.d entries', [switch]$NoCommit) {
  foreach ($name in $Files.Keys) {
    $file = Join-Path $fragmentDir $name
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $file) | Out-Null
    if ($Files[$name] -is [byte[]]) { [IO.File]::WriteAllBytes($file, $Files[$name]) } else { [IO.File]::WriteAllText($file, $Files[$name]) }
  }
  if ($NoCommit) { return }
  G $work (@('add', '--') + @($Files.Keys | ForEach-Object { "changelog.d/$_" })) | Out-Null
  G $work @('commit', '--quiet', '-m', "sandbox: $Message") | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
}
# Removes these changelog.d files, or every one but its README, in one pushed commit.
function Clear-Fragments([string[]]$Names) {
  if (-not $Names) { $Names = @(Get-ChildItem -LiteralPath $fragmentDir -File | Where-Object { $_.Name -cne 'README.md' } | ForEach-Object { $_.Name }) }
  # Only those still tracked: a release that went ahead when its check failed has deleted them already.
  $paths = @($Names | ForEach-Object { "changelog.d/$_" } | Where-Object { G $work @('ls-files', '--', $_) })
  if (-not $paths.Count) { return }
  G $work (@('rm', '--quiet', '--') + $paths) | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: clear changelog.d') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
}
# changelog.d and CHANGELOG.md as bytes, to tell that a run left them byte for byte as they were.
function Get-FileSnapshot {
  $files = @(Get-ChildItem -LiteralPath $fragmentDir -File | Sort-Object Name) + @(Get-Item -LiteralPath (Join-Path $work 'CHANGELOG.md'))
  ($files | ForEach-Object { "$($_.Name)=$([Convert]::ToBase64String([IO.File]::ReadAllBytes($_.FullName)))" }) -join ';'
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
  # The scenarios count every changelog.d entry, so the ones this checkout's HEAD has pending stay out of the sandbox.
  G $work @('rm', '-r', '--quiet', '--ignore-unmatch', '--', 'changelog.d') | Out-Null
  if (Test-Path -LiteralPath $fragmentDir) { Remove-Item -LiteralPath $fragmentDir -Recurse -Force }
  New-Item -ItemType Directory -Force -Path $fragmentDir | Out-Null
  Copy-Item -LiteralPath (Join-Path $repo 'changelog.d' 'README.md') -Destination $fragmentDir -Force
  # git checks changelog.d out with CRLF wherever it runs, as core.autocrlf does on Windows, so putting a file with LF
  # back byte for byte takes more than git restore.
  [IO.File]::WriteAllText((Join-Path $work '.gitattributes'), "changelog.d/*.md text eol=crlf`n")
  [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "1.6.4`n")
  # CRLF, as a Windows checkout has it: the release must keep the file's line endings.
  [IO.File]::WriteAllText((Join-Path $work 'CHANGELOG.md'), ($fixture -replace "`n", "`r`n"))
  G $work (@('add', '--') + @($scripts | ForEach-Object { "build/$_" }) + @('VERSION', 'CHANGELOG.md', '.gitattributes', 'changelog.d/README.md')) | Out-Null
  G $work @('commit', '--quiet', '--allow-empty', '-m', 'sandbox: the release script under test') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  $held = (@(Get-ChildItem -LiteralPath $fragmentDir -Force | ForEach-Object { $_.Name }) -join ',') + ' | ' + (G $work @('ls-files', '--', 'changelog.d'))
  Check 'the sandbox changelog.d holds only its README, on disk and in git' ($held -ceq 'README.md | changelog.d/README.md') $held
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
  Check 'a tag without its section is refused' ($r.Code -eq 1 -and $r.Text -match 'CHANGELOG\.md has no \[1\.6\.4\] heading: the release would have no notes') $r.Text
  $changelogPath = Join-Path $work 'CHANGELOG.md'
  $text = Get-Content -LiteralPath $changelogPath -Raw
  $nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
  [IO.File]::WriteAllText($changelogPath, $text.Replace("## [Unreleased]$nl", "## [Unreleased]$nl$nl<!--$nl## [1.6.4] - 2026-01-01$nl$nl- **Old.** One line.$nl-->$nl"))
  $r = Invoke-Release @('1', '1.6.4', 'y', 'y') @('-DryRun')
  Check '  and one inside a comment is named as the cause' ($r.Text -match 'no \[1\.6\.4\] heading \(it is inside an HTML comment or a code fence\)' -and
    $r.Text -notmatch 'no entries under \[1\.6\.4\]') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
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
  Check 'a comment that never closes warns' ($r.Code -eq 0 -and $r.Text -match '\[!!\] CHANGELOG\.md has an HTML comment or a code fence that never closes' -and
    $r.Text -notmatch 'notes never closes') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  # Inside [Unreleased], after an entry, it would take the rest of the file into the notes: that stops a release.
  $text = Get-Content -LiteralPath $file -Raw
  [IO.File]::WriteAllText($file, $text.Replace("- **Second fix.** One line.$nl", "- **Second fix.** One line.$nl<!-- a comment that never closes$nl"))
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check '  inside the notes, it is the stop that the notes would run to the end' ($r.Code -eq 0 -and
    $r.Text -match '\[!!\] an HTML comment or a code fence in the \[[0-9.]+\] notes never closes, so they would run to the end of CHANGELOG\.md') $r.Text
  G $work @('checkout', '--quiet', '--', 'CHANGELOG.md') | Out-Null
  # Above [Unreleased] the same comment hides the heading: the warning comes before the stop it causes.
  $text = Get-Content -LiteralPath $file -Raw
  [IO.File]::WriteAllText($file, $text.Replace('## [Unreleased]', "<!-- a comment that never closes$nl$nl## [Unreleased]"))
  $r = Invoke-Release @('4', '', 'y') @('-DryRun')
  Check '  above [Unreleased], it warns before the missing-heading stop' ($r.Code -eq 1 -and
    $r.Text -match '(?s)\[!!\] CHANGELOG\.md has an HTML comment or a code fence that never closes.*\[xx\] CHANGELOG\.md has no "## \[Unreleased\]" heading') $r.Text
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
  $hiddenAbove = Join-Path $Sandbox 'hidden-above.md'
  [IO.File]::WriteAllText($hiddenAbove, "<!-- never closed`n`n## [4.0.0] - 2026-03-01`n`n- **Entry.** One line.`n")
  $n = Invoke-Notes @('-Version', '4.0.0', '-Changelog', $hiddenAbove)
  Check 'a section hidden by a comment above it fails, naming the cause' ($n.Code -eq 1 -and
    $n.Text -match 'no \[4\.0\.0\] section \(its heading is inside an HTML comment or a code fence\)') $n.Text
  [IO.File]::WriteAllText($hiddenAbove, "## [4.0.0] - 2026-03-01`n`n- **Entry.** One line.`n<!-- never closed`n`n## [3.0.0] - 2026-02-01`n`n- **Old.** One line.`n")
  $n = Invoke-Notes @('-Version', '4.0.0', '-Changelog', $hiddenAbove)
  Check 'a comment left open after an entry fails: the notes would run to the end' ($n.Code -eq 1 -and
    $n.Text -match "\[4\.0\.0\] section never closes, so its notes would run to the end of the file") $n.Text
  [IO.File]::WriteAllText($hiddenAbove, "<!-- never closed`n`n## [5.0.0] - 2026-04-01`n`n- **Entry.** One line.`n`n## [4.0.0] - 2026-03-01`n`n- x`n")
  $n = Invoke-Notes @('-Version', '9.9.9', '-Changelog', $hiddenAbove)
  Check 'an unrelated comment left open names no cause for a missing section' ($n.Code -eq 1 -and $n.Text -match 'no \[9\.9\.9\] section:') $n.Text
  [IO.File]::WriteAllText($hiddenAbove, "## [4.0.0] - 2026-03-01`n`n### Fixed`n`n- <!-- describe the fix here -->`n")
  $n = Invoke-Notes @('-Version', '4.0.0', '-Changelog', $hiddenAbove)
  Check 'a placeholder entry inside an inline comment is no entry' ($n.Code -eq 1 -and $n.Text -match '\[4\.0\.0\] section has no entries') $n.Text

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

  # 14e. Leaving the final's review keeps the edits made there. Notes it couldn't post are asked about again: the
  #      heading gone (or inside a comment), only a commented-out placeholder left, or a comment left open after an
  #      entry. n copies the edited CHANGELOG.md out of the repo before it puts the files back.
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
    $written = Get-Content -LiteralPath $file -Raw
    [IO.File]::WriteAllText($file, ($written -replace '(?m)^## \[1\.6\.7\] - .*\r?\n', ''))
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText($file, ($written -replace '(?m)^(## \[1\.6\.7\] - )', "<!-- never closed`r`n`r`n`$1"))
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText($file, ($written -replace '(?s)(## \[1\.6\.7\] - [^\r\n]*\r\n).*?(?=## \[|\[Unreleased\]: )', "`$1`r`n- <!-- describe the fix here -->`r`n`r`n"))
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText($file, ($written -replace '(?s)(## \[1\.6\.7\] - .*?\r\n- [^\r\n]*\r\n)', "`$1<!-- never closed`r`n"))
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'a review edit that removes the heading is asked about again' ($r.Code -eq 1 -and $r.Text -match 'CHANGELOG\.md has no \[1\.6\.7\] heading now: put it back') $r.Text
  Check '  naming a comment around it' ($r.Text -match 'no \[1\.6\.7\] heading now \(it is inside an HTML comment or a code fence\)') $r.Text
  Check '  a placeholder inside a comment is no entry' ($r.Text -match 'CHANGELOG\.md has no entries under \[1\.6\.7\] now: the release would have no notes') $r.Text
  Check '  a comment left open after an entry would take the rest of the file' ($r.Text -match
    'in the \[1\.6\.7\] notes never closes, so they would run to the end of CHANGELOG\.md \(close it, or answer n\)') $r.Text
  $kept = if ($r.Text -match '(?m)Your edits to CHANGELOG\.md are kept in (.+?)\r?$') { $Matches[1] } else { $null }
  $keptText = if ($kept -and (Test-Path -LiteralPath $kept)) { Get-Content -LiteralPath $kept -Raw } else { '' }
  Check '  and n keeps the edited CHANGELOG.md outside the repo' ($keptText -match 'Candidate feature' -and $keptText -match '<!-- never closed') $r.Text
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
  Check 'tag names: release and candidate tags in the spelling the script cuts, nothing else' ((Test-ChangelogTagName 'v1.6.4') -and
    (Test-ChangelogTagName 'v1.7.0-rc.2') -and -not (@('V1.6.4', 'v1.7.0-RC.2', 'bf77a8f', '', 'v1.6', 'v1.6.4U', "v1.6.4`n") | Where-Object { Test-ChangelogTagName $_ })) ''
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
  # Inside an entry, a comment hides text up to its "-->" within the paragraph; one still open at the paragraph's end
  # shows as text. (Each case below matches GitHub's rendering of it.)
  $counts = Get-ChangelogEntryCounts "### Fixed`n`n- <!-- describe the fix here -->`n- <!-->`n`n### Added`n`n- <!-- a --> real <!-- b -->`n- a <!-- open`n- c -->"
  Check 'inline comments: a placeholder or "<!-->" is no entry; text beside one, or after one left open, is' ($counts.Count -eq 1 -and $counts['Added'] -eq 3) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  Check '  one over two lines of an entry is no wrapped line' (-not (Get-ChangelogWrappedLines "- **Fix.** Text <!-- a note`n  that wraps --> more.")) ''
  $wrapped = @(Get-ChangelogWrappedLines "- b <!-- x`n`n  y -->")
  Check '  but one over a blank line is no comment' ($wrapped.Count -eq 1 -and $wrapped[0] -eq '  y -->') ($wrapped -join '|')
  $wrapped = @(Get-ChangelogWrappedLines "- a <!-- x`n<!-- note -->`n  b -->")
  Check '  and neither is one over a comment block' ($wrapped.Count -eq 1 -and $wrapped[0] -eq '  b -->') ($wrapped -join '|')
  # A list item whose text starts with "<!--" opens a comment block, which ends with the item: left open there, it
  # hides the rest of the file. One indented into an item does the same; one at the margin is outside the list.
  $section = "- <!-- describe the fix here,`n`n  over a blank line -->`n- <!-- x`n  y -->"
  $counts = Get-ChangelogEntryCounts $section
  Check 'an item opening a comment: closed inside the item, it hides the item' ($counts.Count -eq 0 -and -not (Test-ChangelogUnclosed $section) -and
    -not (Get-ChangelogWrappedLines $section)) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  Check '  left open when the item ends, it never closes' ((Test-ChangelogUnclosed "- real`n- <!-- open`n- next -->") -and
    (Test-ChangelogUnclosed "- <!-- x`n$fence3`nin a fence`n$fence3`n  b -->") -and (Test-ChangelogUnclosed "- a`n  - <!-- x`n  - b -->`n- c") -and
    (Test-ChangelogUnclosed "### Fixed`n`n- real`n- <!-- open`n`n## [1.0.0]`n`n- old")) ''
  $counts = Get-ChangelogEntryCounts "- a`n`n  <!-- x`n`n  y -->`n- b`n<!-- z`n- hidden -->`n- c"
  Check '  one indented into an item closes inside it; one at the margin is outside the list' ($counts['(no group)'] -eq 3 -and
    -not (Test-ChangelogUnclosed "- a`n`n  <!-- x`n`n  y -->`n- b") -and (Test-ChangelogUnclosed "- a`n  <!-- note`n- b -->`n- c")) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  # A fence inside an item closes with the item, so a closing fence at the margin opens a new one; a nested item's
  # fence is indented past its text, not four columns.
  $counts = Get-ChangelogEntryCounts "- a`n  - b`n`n    $fence3`n    <!-- x`n    $fence3`n- c"
  Check 'a fence in a nested item hides its lines, comments included' ($counts['(no group)'] -eq 2 -and
    -not (Test-ChangelogUnclosed "- a`n  - b`n`n    $fence3`n    <!-- x`n    $fence3`n- c")) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  $counts = Get-ChangelogEntryCounts "- a`n  $fence3`n  x`n- b"
  Check '  one left open closes with its item' ($counts['(no group)'] -eq 2 -and -not (Test-ChangelogUnclosed "- a`n  $fence3`n  x`n- b")) (($counts.Keys | ForEach-Object { "$_=$($counts[$_])" }) -join ',')
  Check '  and its fence at the margin opens a new one, which never closes' (Test-ChangelogUnclosed "- a`n  $fence3`n  code`n$fence3`n- b`n`n### Fixed`n`n- c") ''
  Check 'a heading inside a comment is a hidden heading; a visible one, or none, is not' ((Test-ChangelogHeadingHidden "<!--`n## [1.0.0]`n-->" '1.0.0') -and
    -not (Test-ChangelogHeadingHidden "<!--`n## [1.0.0]`n-->`n## [1.0.0]" '1.0.0') -and -not (Test-ChangelogHeadingHidden "## [2.0.0]" '1.0.0')) ''
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

  # 15. changelog.d: one file per entry, added to [Unreleased] at a release and deleted by its commit.
  # 15a. The addition on its own: after an existing group's entries (its heading in any case), new headings in Keep a
  #      Changelog order ahead of a group it doesn't know, nothing taken from the next section, a hidden heading
  #      passed over, an empty group, an empty section, CRLF kept.
  function E([string]$Group, [string]$Text) { [pscustomobject]@{ Group = $Group; Text = $Text } }
  $out = Add-ChangelogUnreleased -Text "## [Unreleased]`n`n### fixed`n`n- old`n`n### Notes`n`n- n`n`n## [1.0.0] - D`n`n### Added`n`n- x`n" -Entries @(
    (E 'Fixed' '- f1'), (E 'Security' '- s1'), (E 'Added' '- a1'), (E 'Fixed' '- f2'))
  Check 'entries follow a group''s own, and new groups go in Keep a Changelog order before unknown ones' ($out -ceq
    "## [Unreleased]`n`n### Added`n`n- a1`n`n### fixed`n`n- old`n- f1`n- f2`n`n### Security`n`n- s1`n`n### Notes`n`n- n`n`n## [1.0.0] - D`n`n### Added`n`n- x`n") $out
  $out = Add-ChangelogUnreleased -Text ("## [Unreleased]`n`n<!--`n### Fixed`n-->`n`n### Security`n`n## [1.0.0] - D`n`n- x`n" -replace "`n", "`r`n") -Entries @(
    (E 'Fixed' '- f1'), (E 'Security' '- s1'))
  Check '  past a commented-out heading, into an empty group, with CRLF' ($out -ceq
    ("## [Unreleased]`n`n<!--`n### Fixed`n-->`n`n### Fixed`n`n- f1`n`n### Security`n`n- s1`n`n## [1.0.0] - D`n`n- x`n" -replace "`n", "`r`n")) $out
  $out = Add-ChangelogUnreleased -Text '## [Unreleased]' -Entries @((E 'Changed' '- c1'))
  Check '  and into an empty section at the end of the file' ($out -ceq "## [Unreleased]`n`n### Changed`n`n- c1") $out
  # One file's text as an entry: what stops it, and the line breaks at its end, which don't.
  $textCases = [ordered]@{
    'empty' = @('', 'is empty')
    'a lone newline' = @("`n", 'is empty')
    'blank lines only' = @("`r`n`n", 'is empty')
    'an entry and blank lines after it' = @("- **A.** x`r`n`r`n`n", $null)
    'a blank line inside' = @("- **A.** x`n`n- **B.** y`n", 'has more than one line')
    'spaces on a line after it' = @("- **A.** x`n  `r`n", $null)
    'a comment and nothing else' = @('- <!-- x -->', 'is no entry once its HTML comments are hidden')
    'a comment that never closes' = @('- <!-- x', 'opens an HTML comment or a code fence that never closes')
    'a leading space' = @(' - **A.** x', 'does not open with "- "')
    'a leading tab' = @("`t- **A.** x", 'does not open with "- "')
    'a "*" item' = @('* **A.** x', 'does not open with "- "')
    'a backtick fence' = @('- ```', 'opens a code fence after "- "')
    'a tilde fence' = @('-  ~~~ x', 'opens a code fence after "- "')
    'inline code of three backticks' = @('- ```js x```', $null)
    'a tilde fence after a tab' = @("- `t~~~", 'opens a code fence after "- "')
    'inline code of three backticks after a tab' = @("- `t``````js x``````", $null)
    'a NUL in place of a space' = @("- **A.**$([char]0)x", 'holds a control character')
    'a right-to-left override' = @("- **A.** x $([char]0x202E)y", 'holds a control character')
    'a tab inside' = @("- **A.**`tx", $null)
  }
  foreach ($case in $textCases.Keys) {
    $text, $expect = $textCases[$case]
    $got = Get-ChangelogFragmentProblem $text
    $pass = if ($expect) { "$got".StartsWith($expect, [StringComparison]::Ordinal) } else { $null -eq $got }
    Check "changelog.d text, ${case}: $(if ($expect) { $expect } else { 'an entry' })" $pass "$got"
  }
  # Its bytes: strict UTF-8, a BOM dropped without a word.
  $utf8 = [Text.UTF8Encoding]::new($false)
  $dash = [string][char]0x2014
  $d = ConvertFrom-ChangelogFragmentBytes ([byte[]](@(0xEF, 0xBB, 0xBF) + $utf8.GetBytes("- **A.** x $dash y`n")))
  Check 'a changelog.d file with a UTF-8 BOM reads without it' ($d.Text -ceq "- **A.** x $dash y`n" -and $null -eq $d.Problem) "$($d.Text)|$($d.Problem)"
  $d = ConvertFrom-ChangelogFragmentBytes ([byte[]]($utf8.GetBytes('- **A.** x ') + @(0x97, 0x0A)))
  Check '  one in Windows-1252 is not valid UTF-8' ($d.Problem -ceq 'is not valid UTF-8 (save it as UTF-8)') "$($d.Problem)"
  $d = ConvertFrom-ChangelogFragmentBytes ($utf8.GetBytes("- **A.** x $([char]0xFFFD) y`n"))
  Check '  and one holding U+FFFD is refused' ("$($d.Problem)".StartsWith('holds U+FFFD', [StringComparison]::Ordinal)) "$($d.Problem)"

  # 15b. Entries from several commits, one a merge whose branch commit is older than the commit before the merge:
  #      each group's go in the order their files reached master's first-parent history (where each was added, not
  #      last changed), files added together by name, an uncommitted one last. A dry run shows them and changes
  #      nothing.
  Add-Entry '- **Existing fix.** One line.'
  Add-Fragments ([ordered]@{ 'zeta.fixed.md' = "- **Zeta fix.** One line.`n" }) 'zeta'
  Add-Fragments ([ordered]@{ 'beta.fixed.md' = "- **Beta fix.** One line.`r`n"; 'alpha.fixed.md' = '- **Alpha fix.** One line.'
      'gamma.added.md' = "- **Gamma feature.** One line.`n" }) 'alpha, beta and gamma'
  G $work @('checkout', '--quiet', '-b', 'side') | Out-Null
  Add-Fragments ([ordered]@{ 'aaa-side.fixed.md' = "- **Side fix.** One line.`n" }) -NoCommit
  $env:GIT_AUTHOR_DATE = '2001-01-01T00:00:00Z'
  $env:GIT_COMMITTER_DATE = '2001-01-01T00:00:00Z'
  try {
    G $work @('add', '--', 'changelog.d/aaa-side.fixed.md') | Out-Null
    G $work @('commit', '--quiet', '-m', 'sandbox: an older commit on a branch') | Out-Null
  } finally { $env:GIT_AUTHOR_DATE = $null; $env:GIT_COMMITTER_DATE = $null }
  G $work @('checkout', '--quiet', 'master') | Out-Null
  Add-Fragments ([ordered]@{ 'mmm.fixed.md' = "- **Mmm fix.** One line.`n"; 'sec.security.md' = "- **Sec fix.** One line.`n" }) 'mmm and sec'
  G $work @('merge', '--quiet', '--no-ff', '--no-edit', '-m', 'sandbox: merge side', 'side') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  G $work @('branch', '--quiet', '-D', 'side') | Out-Null
  # A later commit that changes a file leaves its entry where the file was added.
  Add-Fragments ([ordered]@{ 'zeta.fixed.md' = '- **Zeta fix.** One line.' }) 'zeta without its newline'
  Add-Fragments ([ordered]@{ 'aaa-new.fixed.md' = "- **New fix.** One line.`n" }) -NoCommit
  $fixed = '- **Existing fix.** One line.', '- **Zeta fix.** One line.', '- **Alpha fix.** One line.', '- **Beta fix.** One line.',
    '- **Mmm fix.** One line.', '- **Side fix.** One line.'
  $want = "### Added`n`n- **Gamma feature.** One line.`n`n### Fixed`n`n$($fixed -join "`n")`n- **New fix.** One line.`n`n### Security`n`n- **Sec fix.** One line."
  $before = Get-FileSnapshot
  $status = G $work @('status', '--porcelain', '--untracked-files=all')
  $head = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  $shown = ($want -split "`n" | ForEach-Object { "    $_" }) -join "`n"
  Check 'a dry run shows [Unreleased] with the changelog.d entries in the order they reached master' ($r.Code -eq 0 -and
    $r.Text.Contains("  [Unreleased] with them:`n$shown`n")) $r.Text
  $order = 'Fixed: changelog.d/zeta.fixed.md', 'Fixed: changelog.d/alpha.fixed.md', 'Fixed: changelog.d/beta.fixed.md',
    'Added: changelog.d/gamma.added.md', 'Fixed: changelog.d/mmm.fixed.md', 'Security: changelog.d/sec.security.md',
    'Fixed: changelog.d/aaa-side.fixed.md', 'Fixed: changelog.d/aaa-new.fixed.md'
  Check '  and lists them in that order' ($r.Text.Contains("changelog.d: 8 entries for [Unreleased], in this order:`n" + (($order | ForEach-Object { "      $_" }) -join "`n"))) $r.Text
  Check '  counts them' ($r.Text -match 'CHANGELOG\.md \[Unreleased\]: Added 1, Fixed 7, Security 1, changelog\.d included') $r.Text
  Check '  plans to delete them in the release commit' ($r.Text -match 'deleting the 8 changelog\.d files, as "chore\(release\): v1\.6\.8-rc\.1"' -and
    $r.Text -match '> git rm --quiet -- changelog\.d/zeta\.fixed\.md changelog\.d/alpha\.fixed\.md ') $r.Text
  Check '  warns that the uncommitted one would stop a release' ($r.Text -match '\[!!\] changelog\.d: changelog\.d/aaa-new\.fixed\.md: not committed' -and
    $r.Text -match '\[!!\] changelog\.d has 1 problem\(s\)') $r.Text
  Check '  and changes no file, commit or tag' ((Get-FileSnapshot) -ceq $before -and (G $work @('status', '--porcelain', '--untracked-files=all')) -ceq $status -and
    (G $work @('rev-parse', 'HEAD')) -eq $head -and -not (G $work @('tag', '--list', 'v1.6.8-rc.1'))) ''

  # 15b2. In a shallow clone whose oldest commit (the merge) already holds every file, they all count as committed
  #       and go in by name, with log.showRoot off as well.
  $shallow = Join-Path $Sandbox 'shallow'
  & git clone --quiet --depth 2 --branch master "file:///$($origin -replace '\\', '/')" $shallow
  if ($LASTEXITCODE) { throw 'shallow clone failed' }
  G $shallow @('config', 'log.showRoot', 'false') | Out-Null
  $out = (@('1', '', 'y') -join "`n") | pwsh -NoProfile -File (Join-Path $shallow 'build' 'Release-D2BS.ps1') -DryRun -NoFetch 2>&1
  $r = [pscustomobject]@{ Code = $LASTEXITCODE; Text = (@($out) -join "`n") }
  $order = 'Fixed: changelog.d/aaa-side.fixed.md', 'Fixed: changelog.d/alpha.fixed.md', 'Fixed: changelog.d/beta.fixed.md',
    'Added: changelog.d/gamma.added.md', 'Fixed: changelog.d/mmm.fixed.md', 'Security: changelog.d/sec.security.md', 'Fixed: changelog.d/zeta.fixed.md'
  Check 'a shallow clone folds every committed file, by name, none of them read as not committed' ((G $shallow @('rev-parse', '--is-shallow-repository')) -eq 'true' -and
    $r.Code -eq 0 -and $r.Text -notmatch 'not committed' -and
    $r.Text.Contains("changelog.d: 7 entries for [Unreleased], in this order:`n" + (($order | ForEach-Object { "      $_" }) -join "`n"))) $r.Text
  Remove-Item -LiteralPath $shallow -Recurse -Force

  # 15c. A real release stops at the plan on the uncommitted file, which its commit could not delete.
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'an uncommitted changelog.d file stops the release at the plan' ($r.Code -eq 1 -and $r.Text -match '\[xx\] changelog\.d has 1 problem\(s\)' -and
    $r.Text -notmatch 'Proceed\?' -and (Get-FileSnapshot) -ceq $before -and -not (G $work @('tag', '--list', 'v1.6.8-rc.1'))) $r.Text
  Remove-Item -LiteralPath (Join-Path $fragmentDir 'aaa-new.fixed.md')
  $want = "### Added`n`n- **Gamma feature.** One line.`n`n### Fixed`n`n$($fixed -join "`n")`n`n### Security`n`n- **Sec fix.** One line."

  # 15d. An edit to a changelog.d file while "Proceed?" waits stops the release, and the edit survives.
  $alpha = Join-Path $fragmentDir 'alpha.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    [IO.File]::WriteAllText($alpha, '- **Alpha fix, edited.** One line.')
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'an edit to a changelog.d file at Proceed? stops the release' ($r.Code -eq 1 -and $r.Text -match 'changelog\.d or HEAD changed during the prompts' -and
    [IO.File]::ReadAllText($alpha) -ceq '- **Alpha fix, edited.** One line.' -and -not (G $work @('tag', '--list', 'v1.6.8-rc.1'))) $r.Text
  [IO.File]::WriteAllText($alpha, '- **Alpha fix.** One line.')

  # 15e. The review names the files the commit deletes and asks again while one is edited; n, or the end of the
  #      input, puts CHANGELOG.md and every file back byte for byte, LF ones included (git would check them out with
  #      CRLF here), keeping the edit outside the repo, and with nothing left for git status to show.
  $before = Get-FileSnapshot
  $zeta = Join-Path $fragmentDir 'zeta.fixed.md'
  $beta = Join-Path $fragmentDir 'beta.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText($zeta, "- **Zeta fix, edited at the review.** One line.`n")
    Remove-Item -LiteralPath $beta
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'the review names the files the commit deletes' (($r.Text -replace "`r`n", "`n").Contains("The release commit deletes the 7 changelog.d files, whose entries are in CHANGELOG.md now:`n" +
    "      changelog.d/zeta.fixed.md`n      changelog.d/alpha.fixed.md`n")) $r.Text
  Check '  and asks again while one is edited' ($r.Text -match 'changelog\.d/zeta\.fixed\.md changed during the review, and the commit deletes it: make the edit in CHANGELOG\.md, or answer n' -and
    $r.Text -notmatch 'beta\.fixed\.md changed') $r.Text
  Check 'n at the review puts CHANGELOG.md and every changelog.d file back byte for byte' ($r.Code -eq 1 -and (Get-FileSnapshot) -ceq $before -and
    -not (G $work @('status', '--porcelain', '--untracked-files=all'))) "$($r.Text)`n$(G $work @('status', '--porcelain', '--untracked-files=all'))"
  Check '  and says so' ($r.Text -match 'VERSION put back to 1\.6\.7, and CHANGELOG\.md with it, and the 7 changelog\.d files as they were') $r.Text
  $kept = if ($r.Text -match '(?m)Your edits to changelog\.d/zeta\.fixed\.md are kept in (.+?)\r?$') { $Matches[1] } else { $null }
  Check '  keeping the edit outside the repo' ($kept -and (Test-Path -LiteralPath $kept) -and [IO.File]::ReadAllText($kept) -match 'edited at the review') $r.Text
  if ($kept -and (Test-Path -LiteralPath $kept)) { Remove-Item -LiteralPath $kept }
  $r = Invoke-Release @('1', '', 'y')
  Check '  as the end of input there does' ($r.Code -eq 1 -and $r.Text -match 'no more input' -and (Get-FileSnapshot) -ceq $before -and
    -not (G $work @('status', '--porcelain'))) $r.Text

  # 15e2. A changelog.d file renamed or added during the review would stay behind with its entry in CHANGELOG.md, so
  #       y asks again while one is there.
  $head = G $work @('rev-parse', 'HEAD')
  $renamed = Join-Path $fragmentDir 'alpha.added.md'
  $extra = Join-Path $fragmentDir 'extra.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Move-Item -LiteralPath $alpha -Destination $renamed
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Move-Item -LiteralPath $renamed -Destination $alpha
    [IO.File]::WriteAllText($extra, "- **Extra fix.** One line.`n")
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } catch {
    # A y that went ahead ends the script before the next prompt: record it as these checks failing, files restored.
    $r = [pscustomobject]@{ Code = -1; Text = "the run stopped early: $_" }
    if (Test-Path -LiteralPath $renamed) { Move-Item -LiteralPath $renamed -Destination $alpha }
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  $changedNote = 'changelog\.d changed during the review \({0}\), and the commit deletes only the files the plan named'
  Check 'y at the review asks again while a changelog.d file is renamed' ($r.Text -match ($changedNote -f 'changelog\.d/alpha\.added\.md')) $r.Text
  Check '  or added' ($r.Text -match ($changedNote -f 'changelog\.d/extra\.fixed\.md')) $r.Text
  Check '  and n then commits nothing' ($r.Code -eq 1 -and (G $work @('rev-parse', 'HEAD')) -eq $head -and -not (G $work @('tag', '--list', 'v1.6.8-rc.1'))) $r.Text
  Remove-Item -LiteralPath $extra
  Check '  leaving the files as they were' ((Get-FileSnapshot) -ceq $before -and -not (G $work @('status', '--porcelain', '--untracked-files=all'))) (G $work @('status', '--porcelain', '--untracked-files=all'))

  # 15e3. Each of the review's checks catches what the other two can't see: an edit git status skips (the file marked
  #       assume-unchanged), a new file git ignores, and a staged edit whose working copy has its bytes back.
  $before = Get-FileSnapshot
  $exclude = Join-Path $work '.git' 'info' 'exclude'
  $excludeBytes = if (Test-Path -LiteralPath $exclude) { [IO.File]::ReadAllBytes($exclude) } else { $null }
  $ignored = Join-Path $fragmentDir 'ignored.fixed.md'
  $zetaBytes = [IO.File]::ReadAllBytes($zeta)
  $alphaBytes = [IO.File]::ReadAllBytes($alpha)
  $steps = @{}
  function Get-FragmentStatusNow { G $work @('status', '--porcelain', '--untracked-files=all', '--', 'changelog.d') }
  G $work @('update-index', '--assume-unchanged', '--', 'changelog.d/zeta.fixed.md') | Out-Null
  [IO.File]::AppendAllText($exclude, "`n/changelog.d/ignored.fixed.md`n")
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    [IO.File]::WriteAllText($zeta, "- **Zeta fix, edited where git status can't see it.** One line.`n")
    $steps['bytes quiet'] = -not (Get-FragmentStatusNow)
    $at = $run.Out.Length
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    $steps['bytes'] = $run.Out.ToString().Substring($at)
    [IO.File]::WriteAllBytes($zeta, $zetaBytes)
    [IO.File]::WriteAllText($ignored, "- **Ignored fix.** One line.`n")
    $steps['names quiet'] = -not (Get-FragmentStatusNow)
    $at = $run.Out.Length
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    $steps['names'] = $run.Out.ToString().Substring($at)
    Remove-Item -LiteralPath $ignored
    [IO.File]::WriteAllText($alpha, '- **Alpha fix, staged.** One line.')
    G $work @('add', '--', 'changelog.d/alpha.fixed.md') | Out-Null
    [IO.File]::WriteAllBytes($alpha, $alphaBytes)
    $at = $run.Out.Length
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    $steps['status'] = $run.Out.ToString().Substring($at)
    G $work @('restore', '--staged', '--', 'changelog.d/alpha.fixed.md') | Out-Null
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } catch {
    # A y that went ahead ends the script before the next prompt: record it as these checks failing.
    $r = [pscustomobject]@{ Code = -1; Text = "the run stopped early: $_" }
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
    # Still tracked only if the release stopped as it should; otherwise its commit deleted the file.
    if (G $work @('ls-files', '--', 'changelog.d/zeta.fixed.md')) { G $work @('update-index', '--no-assume-unchanged', '--', 'changelog.d/zeta.fixed.md') | Out-Null }
    if ($null -eq $excludeBytes) { Remove-Item -LiteralPath $exclude -ErrorAction SilentlyContinue } else { [IO.File]::WriteAllBytes($exclude, $excludeBytes) }
    if (Test-Path -LiteralPath $ignored) { Remove-Item -LiteralPath $ignored }
  }
  Check 'y at the review asks again for an edit git status does not show (the byte check)' ($steps['bytes quiet'] -eq $true -and
    "$($steps['bytes'])" -match 'changelog\.d/zeta\.fixed\.md changed during the review, and the commit deletes it') "$($steps['bytes'])`n$($r.Text)"
  Check '  for a new file git ignores (the folder listing)' ($steps['names quiet'] -eq $true -and
    "$($steps['names'])" -match ($changedNote -f 'changelog\.d/ignored\.fixed\.md')) "$($steps['names'])`n$($r.Text)"
  Check '  and for a staged edit whose working copy has its bytes back (git status)' ("$($steps['status'])" -match
    'changelog\.d/alpha\.fixed\.md changed during the review, and the commit deletes it') "$($steps['status'])`n$($r.Text)"
  Check '  and n then commits nothing and leaves the files as they were' ($r.Code -eq 1 -and (G $work @('rev-parse', 'HEAD')) -eq $head -and
    (Get-FileSnapshot) -ceq $before -and -not (G $work @('status', '--porcelain', '--untracked-files=all'))) "$($r.Text)`n$(G $work @('status', '--porcelain', '--untracked-files=all'))"

  # 15e4. A changelog.d file locked during the review can be neither compared nor put back: n names it and still puts
  #       VERSION and CHANGELOG.md back.
  $gamma = Join-Path $fragmentDir 'gamma.added.md'
  $lock = $null
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    $lock = [IO.File]::Open($gamma, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::None)
    Send-Answer $run 'n'
    $r = Complete-Release $run
  } finally {
    if ($lock) { $lock.Dispose() }
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'n with a changelog.d file locked names the file it could not put back' ($r.Code -eq 1 -and $r.Text -match 'Could not put changelog\.d/gamma\.added\.md back') $r.Text
  Check '  and still puts VERSION and CHANGELOG.md back' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.7' -and
    -not (G $work @('status', '--porcelain', '--', 'VERSION', 'CHANGELOG.md'))) "$($r.Text)`n$(G $work @('status', '--porcelain'))"
  Check '  leaving every file as it was' ((Get-FileSnapshot) -ceq $before -and -not (G $work @('status', '--porcelain', '--untracked-files=all'))) (G $work @('status', '--porcelain', '--untracked-files=all'))

  # 15e5. A commit that fails (refused by a pre-commit hook here) prints its undo, which works as printed: VERSION,
  #       CHANGELOG.md and every changelog.d file as HEAD has them, and a clean tree.
  $hook = Join-Path $work '.git' 'hooks' 'pre-commit'
  [IO.File]::WriteAllText($hook, "#!/bin/sh`necho sandbox refuses commits`nexit 1`n")
  try { $r = Invoke-Release @('1', '', 'y', 'y') } finally { Remove-Item -LiteralPath $hook }
  $undo = @($r.Text -split "`n" | Where-Object { $_ -cmatch '^  git ' } | ForEach-Object { $_.Trim() })
  $wantUndo = 'git restore --staged --worktree -- VERSION CHANGELOG.md ' + ((@('zeta.fixed', 'alpha.fixed', 'beta.fixed', 'gamma.added', 'mmm.fixed',
        'sec.security', 'aaa-side.fixed') | ForEach-Object { "changelog.d/$_.md" }) -join ' ')
  Check 'a failed commit stops the release and prints its undo' ($r.Code -eq 1 -and $r.Text -match 'git commit failed' -and $undo.Count -eq 1 -and
    $undo[0] -ceq $wantUndo) "$($undo -join ' | ')`n$($r.Text)"
  if ($undo.Count -eq 1) { G $work @($undo[0].Substring(4) -split ' ') | Out-Null }
  $names = @(Get-ChildItem -LiteralPath $fragmentDir -File | ForEach-Object { $_.Name }) -join ','
  Check '  and the undo puts VERSION, CHANGELOG.md and every changelog.d file back as HEAD has them' ((G $work @('rev-parse', 'HEAD')) -eq $head -and
    (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.7' -and -not (G $work @('status', '--porcelain', '--untracked-files=all')) -and
    $names -ceq 'aaa-side.fixed.md,alpha.fixed.md,beta.fixed.md,gamma.added.md,mmm.fixed.md,README.md,sec.security.md,zeta.fixed.md' -and
    -not (G $work @('tag', '--list', 'v1.6.8-rc.1'))) "$names`n$(G $work @('status', '--porcelain', '--untracked-files=all'))"

  # 15f. A release candidate takes them into its own section and its commit deletes them, after printing the undo of
  #      its git steps. A folded file deleted at the review goes with the rest.
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.8-rc.1]: '
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    Send-Answer $run 'y'
    Wait-Prompt $run $review
    Remove-Item -LiteralPath $beta
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check 'a release candidate with changelog.d entries exits 0' ($r.Code -eq 0 -and (G $origin @('tag', '--list', 'v1.6.8-rc.1'))) $r.Text
  Check '  taking a folded file deleted at the review without asking again' ($r.Text -notmatch 'changed during the review' -and
    ([regex]::Matches($r.Text, [regex]::Escape($review))).Count -eq 1) $r.Text
  $undoAt = $r.Text.IndexOf('Should these steps stop before the commit, this undoes them: git restore --staged --worktree -- VERSION CHANGELOG.md changelog.d/zeta.fixed.md changelog.d/alpha.fixed.md ', [StringComparison]::Ordinal)
  $addAt = $r.Text.IndexOf('> git add -- VERSION CHANGELOG.md', [StringComparison]::Ordinal)
  Check '  printing the undo before its first git step' ($undoAt -ge 0 -and $addAt -gt $undoAt) $r.Text
  Check '  its section holds them' ((Read-Changelog).Contains("## [Unreleased]`n`n## [1.6.8-rc.1] - $today`n`n$want`n`n## [1.6.7]")) (Read-Changelog)
  $n = Invoke-Notes @('-Version', 'v1.6.8-rc.1')
  Check '  and Get-ReleaseNotes posts it' ($n.Code -eq 0 -and $n.Text -ceq $want) $n.Text
  $changes = (G $work @('show', '--name-status', '--format=', 'HEAD')) -replace "`t", ' '
  Check '  its commit deletes the files, the one deleted at the review included, with VERSION and CHANGELOG.md' ($changes -ceq ((@('M CHANGELOG.md', 'M VERSION') + @('aaa-side.fixed', 'alpha.fixed',
    'beta.fixed', 'gamma.added', 'mmm.fixed', 'sec.security', 'zeta.fixed' | ForEach-Object { "D changelog.d/$_.md" })) -join "`n")) $changes
  Check '  leaving changelog.d its README and a clean tree' ((@(Get-ChildItem -LiteralPath $fragmentDir -File | ForEach-Object { $_.Name }) -join ',') -ceq 'README.md' -and
    -not (G $work @('status', '--porcelain', '--untracked-files=all'))) (G $work @('status', '--porcelain', '--untracked-files=all'))

  # 15g. The final merges the candidate's section with the entries added since, as it merges any. A slug the
  #      candidate's commit deleted and a later commit used again sorts where it came back.
  Add-Fragments ([ordered]@{ 'late-change.changed.md' = "- **Late change.** One line.`n"; 'late-fix.fixed.md' = "- **Late fix.** One line.`n" }) 'late entries'
  Add-Fragments ([ordered]@{ 'alpha.fixed.md' = "- **Alpha again.** One line.`n" }) 'alpha again'
  $r = Invoke-Release @('2', '', 'y', 'y')
  $want = ("### Added`n`n- **Gamma feature.** One line.`n`n### Changed`n`n- **Late change.** One line.`n`n### Fixed`n`n$($fixed -join "`n")`n" +
    "- **Late fix.** One line.`n- **Alpha again.** One line.`n`n### Security`n`n- **Sec fix.** One line.")
  $n = Invoke-Notes @('-Version', '1.6.8')
  Check 'the final merges the candidate and the later changelog.d entries, a slug used again where it came back' ($r.Code -eq 0 -and $n.Text -ceq $want) "$($r.Text)`n---`n$($n.Text)"
  $changes = (G $work @('show', '--name-status', '--format=', 'HEAD')) -replace "`t", ' '
  Check '  drops the candidate section and deletes the files' ((Read-Changelog) -notmatch '1\.6\.8-rc\.' -and
    $changes -ceq "M CHANGELOG.md`nM VERSION`nD changelog.d/alpha.fixed.md`nD changelog.d/late-change.changed.md`nD changelog.d/late-fix.fixed.md") $changes

  # 15h. A re-release names changelog.d when the entries it leaves out wait there.
  Add-Fragments ([ordered]@{ 'ok.fixed.md' = [byte[]](@(0xEF, 0xBB, 0xBF) + $utf8.GetBytes("- **Fine fix.** One line.`n")) }) 'a valid entry, with a BOM'
  $tagCommit = G $work @('rev-parse', 'v1.6.8^{commit}')
  G $work @('push', '--quiet', 'origin', ':refs/tags/v1.6.8') | Out-Null
  G $work @('tag', '-d', 'v1.6.8') | Out-Null
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check 'a re-release warns that the changelog.d entries stay out' ($r.Code -eq 0 -and $r.Text -match '1\) 1\.6\.8 again \(release commit' -and
    $r.Text -match '\[!!\] changelog\.d has entries, and re-releasing v1\.6\.8 posts only \[1\.6\.8\]') $r.Text
  # It commits nothing and leaves changelog.d alone, so a file added there while Proceed? waits doesn't stop it.
  $late = Join-Path $fragmentDir 'late-add.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Send-Answer $run ''
    Wait-Prompt $run 'Proceed? [Y/n] '
    [IO.File]::WriteAllText($late, "- **Late add.** One line.`n")
    Send-Answer $run 'y'
    $r = Complete-Release $run
  } catch {
    $r = [pscustomobject]@{ Code = -1; Text = "the run stopped early: $_" }
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
    if (Test-Path -LiteralPath $late) { Remove-Item -LiteralPath $late }
  }
  Check '  and a file added to changelog.d while its Proceed? waits does not stop it' ($r.Code -eq 0 -and
    $r.Text -notmatch 'changed during the prompts' -and (G $origin @('tag', '--list', 'v1.6.8'))) $r.Text
  if (G $origin @('tag', '--list', 'v1.6.8')) { G $work @('push', '--quiet', 'origin', ':refs/tags/v1.6.8') | Out-Null }
  if (G $work @('tag', '--list', 'v1.6.8')) { G $work @('tag', '-d', 'v1.6.8') | Out-Null }
  G $work @('tag', '-a', 'v1.6.8', '-m', 'D2BS v1.6.8', $tagCommit) | Out-Null
  G $work @('push', '--quiet', 'origin', 'v1.6.8') | Out-Null

  # 15i. Each problem stops a real release at the plan, though a valid entry (its BOM dropped) would let it go on; a
  #      dry run only warns.
  $problemCases = [ordered]@{
    'a malformed name' = @([ordered]@{ 'Bad_Name.fixed.md' = "- **Bad name.** One line.`n" }, 'changelog\.d/Bad_Name\.fixed\.md: not named <slug>\.<section>\.md')
    'an uppercase slug' = @([ordered]@{ 'Bad.fixed.md' = "- **Uppercase slug.** One line.`n" }, 'changelog\.d/Bad\.fixed\.md: not named <slug>\.<section>\.md')
    'an unknown section' = @([ordered]@{ 'misc.notes.md' = "- **Misc.** One line.`n" }, "changelog\.d/misc\.notes\.md: unknown section 'notes'")
    'a multi-line file' = @([ordered]@{ 'two.fixed.md' = "- **Two lines.** One`n  and two.`n" }, 'changelog\.d/two\.fixed\.md: has more than one line')
    'a slug used twice' = @([ordered]@{ 'dup.fixed.md' = "- **Dup fix.** One line.`n"; 'dup.added.md' = "- **Dup feature.** One line.`n" },
      'changelog\.d/dup\.(added|fixed)\.md, changelog\.d/dup\.(added|fixed)\.md: the same slug twice')
    'a file not in UTF-8' = @([ordered]@{ 'cp.fixed.md' = [byte[]]($utf8.GetBytes('- **Dash ') + @(0x97) + $utf8.GetBytes(' here.** One line.') + @(0x0A)) },
      'changelog\.d/cp\.fixed\.md: is not valid UTF-8 \(save it as UTF-8\)')
    'a file holding U+FFFD' = @([ordered]@{ 'lost.fixed.md' = "- **Lost $([char]0xFFFD) dash.** One line.`n" }, 'changelog\.d/lost\.fixed\.md: holds U\+FFFD')
    'a folder' = @([ordered]@{ 'sub/inner.fixed.md' = "- **Inner.** One line.`n" }, 'changelog\.d/sub: a folder, which holds no entry')
  }
  $fineOnly = "  [Unreleased] with them:`n    ### Fixed`n    `n    - **Fine fix.** One line.`n"
  foreach ($case in $problemCases.Keys) {
    $files, $pattern = $problemCases[$case]
    Add-Fragments $files $case
    if ($case -eq 'a multi-line file') {
      $r = Invoke-Release @('1', '', 'y') @('-DryRun')
      Check 'a dry run only warns about a problem, and leaves that file out' ($r.Code -eq 0 -and $r.Text -match "\[!!\] changelog\.d: $pattern" -and
        $r.Text -match '\[!!\] changelog\.d has 1 problem\(s\)' -and $r.Text.Contains($fineOnly)) $r.Text
    }
    if ($case -eq 'a slug used twice') {
      $r = Invoke-Release @('1', '', 'y') @('-DryRun')
      Check 'a dry run leaves both files of a shared slug out' ($r.Code -eq 0 -and $r.Text -match "\[!!\] changelog\.d: $pattern" -and
        $r.Text -notmatch '\*\*Dup ' -and $r.Text.Contains($fineOnly)) $r.Text
    }
    $r = Invoke-Release @('1', '', 'y', 'y')
    Check "$case in changelog.d stops the release at the plan" ($r.Code -eq 1 -and $r.Text -match "\[!!\] changelog\.d: $pattern" -and
      $r.Text -match '\[xx\] changelog\.d has 1 problem\(s\), listed above' -and $r.Text -notmatch 'Proceed\?' -and -not (G $work @('tag', '--list', 'v1.6.9-rc.1')) -and
      -not (G $work @('status', '--porcelain'))) $r.Text
    Clear-Fragments @($files.Keys)
  }
  $oneProblem = '\[xx\] changelog\.d has 1 problem\(s\), listed above'
  # 15i2. A slug is shared whatever the files hold: next to a file of more than one line, the valid one stays out too.
  Add-Fragments ([ordered]@{ 'dup.fixed.md' = "- **Dup fix.** One line.`n"; 'dup.added.md' = "- **Dup feature.** One`n  and two.`n" }) 'a shared slug, one file invalid'
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check 'a slug shared with a file of more than one line is named, and a dry run leaves both out' ($r.Code -eq 0 -and
    $r.Text -match '\[!!\] changelog\.d: changelog\.d/dup\.added\.md, changelog\.d/dup\.fixed\.md: the same slug twice' -and
    $r.Text -match '\[!!\] changelog\.d: changelog\.d/dup\.added\.md: has more than one line' -and $r.Text -notmatch '\*\*Dup ' -and $r.Text.Contains($fineOnly)) $r.Text
  Clear-Fragments @('dup.fixed.md', 'dup.added.md')
  Add-Fragments ([ordered]@{ 'dup.fixed.md' = "- **Dup fix.** One line.`n"; 'dup.notes.md' = "- **Dup note.** One line.`n" }) 'a shared slug, one section unknown'
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check '  and so is one shared with a file of an unknown section' ($r.Code -eq 0 -and
    $r.Text -match '\[!!\] changelog\.d: changelog\.d/dup\.fixed\.md, changelog\.d/dup\.notes\.md: the same slug twice' -and
    $r.Text -match "\[!!\] changelog\.d: changelog\.d/dup\.notes\.md: unknown section 'notes'" -and $r.Text -notmatch '\*\*Dup ' -and $r.Text.Contains($fineOnly)) $r.Text
  Clear-Fragments @('dup.fixed.md', 'dup.notes.md')

  # 15i3. Only a file named README.md is changelog.d's own: a folder of that name is a folder like any other.
  $readme = Join-Path $fragmentDir 'README.md'
  $readmeBytes = [IO.File]::ReadAllBytes($readme)
  G $work @('rm', '--quiet', '--', 'changelog.d/README.md') | Out-Null
  Add-Fragments ([ordered]@{ 'README.md/inner.fixed.md' = "- **Inner.** One line.`n" }) 'a folder named README.md'
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'a folder named README.md in changelog.d stops the release at the plan' ($r.Code -eq 1 -and
    $r.Text -match '\[!!\] changelog\.d: changelog\.d/README\.md: a folder, which holds no entry' -and $r.Text -match $oneProblem -and
    $r.Text -notmatch 'Proceed\?' -and -not (G $work @('tag', '--list', 'v1.6.9-rc.1'))) $r.Text
  Clear-Fragments @('README.md/inner.fixed.md')
  # Removing the folder's last file makes git delete changelog.d itself, so it is made again before the README.
  New-Item -ItemType Directory -Force -Path $fragmentDir | Out-Null
  [IO.File]::WriteAllBytes($readme, $readmeBytes)
  G $work @('add', '--', 'changelog.d/README.md') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: changelog.d/README.md again') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null

  # 15i4. A tracked file missing from the working tree is a problem: one a sparse checkout leaves out (git status shows
  #       it clean), the whole folder left out, or one deleted after the clean-tree check, while the publish prompt
  #       waits.
  Add-Fragments ([ordered]@{ 'sparse.fixed.md' = "- **Sparse fix.** One line.`n" }) 'an entry a sparse checkout leaves out'
  $missingNote = 'changelog\.d/{0}: tracked by git but missing here \(deleted, or outside a sparse checkout\): restore it, or commit its deletion'
  $allTracked = @((G $work @('ls-files', '--', 'changelog.d')) -split "`n" | Where-Object { $_ })
  try {
    G $work @('update-index', '--skip-worktree', '--', 'changelog.d/sparse.fixed.md') | Out-Null
    Remove-Item -LiteralPath (Join-Path $fragmentDir 'sparse.fixed.md')
    $quiet = -not (G $work @('status', '--porcelain', '--untracked-files=all'))
    $r = Invoke-Release @('1', '', 'y') @('-DryRun')
    Check 'a dry run names a tracked file a sparse checkout leaves out, which git status shows clean' ($quiet -and $r.Code -eq 0 -and
      $r.Text -match ("\[!!\] changelog\.d: $missingNote" -f 'sparse\.fixed\.md') -and $r.Text -match '\[!!\] changelog\.d has 1 problem\(s\)') $r.Text
    $r = Invoke-Release @('1', '', 'y', 'y')
    Check '  and it stops a real release at the plan' ($r.Code -eq 1 -and $r.Text -match ($missingNote -f 'sparse\.fixed\.md') -and $r.Text -match $oneProblem -and
      $r.Text -notmatch 'Proceed\?' -and -not (G $work @('tag', '--list', 'v1.6.9-rc.1'))) $r.Text
    G $work (@('update-index', '--skip-worktree', '--') + $allTracked) | Out-Null
    Remove-Item -LiteralPath $fragmentDir -Recurse -Force
    $r = Invoke-Release @('1', '', 'y', 'y')
    Check '  as does the whole folder left out' ($r.Code -eq 1 -and $r.Text -match ($missingNote -f 'sparse\.fixed\.md') -and
      $r.Text -match ($missingNote -f 'ok\.fixed\.md') -and $r.Text -match '\[xx\] changelog\.d has 2 problem\(s\)' -and $r.Text -notmatch 'Proceed\?' -and
      -not (G $work @('tag', '--list', 'v1.6.9-rc.1'))) $r.Text
  } finally {
    # Only the paths still tracked: a release that went ahead here has deleted the files it folded.
    $stillTracked = @((G $work @('ls-files', '--', 'changelog.d')) -split "`n" | Where-Object { $_ })
    if ($stillTracked.Count) { G $work (@('update-index', '--no-skip-worktree', '--') + $stillTracked) | Out-Null }
    G $work @('checkout', '--quiet', '--', 'changelog.d') | Out-Null
  }
  Add-Fragments ([ordered]@{ 'ahead.fixed.md' = "- **Ahead fix.** One line.`n" }) -NoCommit
  G $work @('add', '--', 'changelog.d/ahead.fixed.md') | Out-Null
  G $work @('commit', '--quiet', '-m', 'sandbox: a commit not pushed yet') | Out-Null
  $run = Start-Release
  try {
    Wait-Prompt $run 'Continue and publish these commits? [y/N] '
    Remove-Item -LiteralPath (Join-Path $fragmentDir 'sparse.fixed.md')
    Send-Answer $run 'y'
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.9-rc.1]: '
    Send-Answer $run ''
    $r = Complete-Release $run
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check '  as does one deleted while the publish prompt waits, after the clean-tree check' ($r.Code -eq 1 -and $r.Text -match ($missingNote -f 'sparse\.fixed\.md') -and
    $r.Text -match $oneProblem -and $r.Text -notmatch 'Proceed\?' -and -not (G $work @('tag', '--list', 'v1.6.9-rc.1'))) $r.Text
  G $work @('checkout', '--quiet', '--', 'changelog.d/sparse.fixed.md') | Out-Null
  # An edit there, which git rm would refuse at the commit step, stops it as well.
  $sparse = Join-Path $fragmentDir 'sparse.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Continue and publish these commits? [y/N] '
    [IO.File]::WriteAllText($sparse, "- **Sparse fix, edited.** One line.`n")
    Send-Answer $run 'y'
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.9-rc.1]: '
    Send-Answer $run ''
    $r = Complete-Release $run
  } catch {
    $r = [pscustomobject]@{ Code = -1; Text = "the run stopped early: $_" }
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
  }
  Check '  and one edited while the publish prompt waits stops it too' ($r.Code -eq 1 -and
    $r.Text -match "changelog\.d/sparse\.fixed\.md: changed since its commit \(git status ' M'\): commit the change, or restore the file" -and
    $r.Text -match $oneProblem -and $r.Text -notmatch 'Proceed\?' -and -not (G $work @('tag', '--list', 'v1.6.9-rc.1'))) $r.Text
  G $work @('checkout', '--quiet', '--', 'changelog.d/sparse.fixed.md') | Out-Null
  # A new file staged there is one problem, "not committed", not also a change to a committed file.
  $staged = Join-Path $fragmentDir 'staged.fixed.md'
  $run = Start-Release
  try {
    Wait-Prompt $run 'Continue and publish these commits? [y/N] '
    [IO.File]::WriteAllText($staged, "- **Staged fix.** One line.`n")
    G $work @('add', '--', 'changelog.d/staged.fixed.md') | Out-Null
    Send-Answer $run 'y'
    Wait-Prompt $run 'Choice [1]: '
    Send-Answer $run '1'
    Wait-Prompt $run 'Version [1.6.9-rc.1]: '
    Send-Answer $run ''
    $r = Complete-Release $run
  } catch {
    $r = [pscustomobject]@{ Code = -1; Text = "the run stopped early: $_" }
  } finally {
    if (-not $run.Proc.HasExited) { $run.Proc.Kill($true) }
    G $work @('rm', '--quiet', '--cached', '--ignore-unmatch', '--', 'changelog.d/staged.fixed.md') | Out-Null
    if (Test-Path -LiteralPath $staged) { Remove-Item -LiteralPath $staged }
  }
  Check '  and one added and staged there is the one problem of not being committed' ($r.Code -eq 1 -and
    $r.Text -match 'changelog\.d/staged\.fixed\.md: not committed' -and $r.Text -notmatch 'staged\.fixed\.md: changed since its commit' -and
    $r.Text -match $oneProblem -and $r.Text -notmatch 'Proceed\?') $r.Text
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  Clear-Fragments @('sparse.fixed.md', 'ahead.fixed.md')

  # 15j. With changelog.d holding only its README and nothing under [Unreleased], a release stops as it always has.
  Clear-Fragments
  $r = Invoke-Release @('1', '', 'y')
  Check 'no changelog.d entries and an empty [Unreleased] stop the release as before' ($r.Code -eq 1 -and $r.Text -match $noEntries -and
    $r.Text -match 'a new version needs them there first, or in changelog\.d' -and $r.Text -notmatch 'changelog\.d:') $r.Text
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
