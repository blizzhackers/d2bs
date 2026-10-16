#Requires -Version 7.2
<#
  End-to-end test of build\Release-D2BS.ps1 in a throwaway sandbox: a bare clone of this repo as "origin", and a
  working clone whose master is this checkout's HEAD plus the WORKING COPY of Release-D2BS.ps1 (an uncommitted change
  to the script is what gets tested), with VERSION set to 1.6.4. Each scenario runs the script with piped answers and
  checks the refs and files it leaves. Nothing reaches this repo or GitHub, and the sandbox is deleted afterwards
  (-Keep keeps it). Exit code = number of failed checks.

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
  Copy-Item -LiteralPath (Join-Path $PSScriptRoot '..' 'Release-D2BS.ps1') -Destination (Join-Path $work 'build') -Force
  [IO.File]::WriteAllText((Join-Path $work 'VERSION'), "1.6.4`n")
  G $work @('add', '--', 'build/Release-D2BS.ps1', 'VERSION') | Out-Null
  G $work @('commit', '--quiet', '--allow-empty', '-m', 'sandbox: the release script under test') | Out-Null
  G $work @('push', '--quiet', 'origin', 'master') | Out-Null
  "sandbox master: $(G $work @('log', '-1', '--format=%h %s')), VERSION $((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim())"
}

function Invoke-Scenarios {
  # 1. Dry run from a clean master: offers the patch first, changes nothing.
  $head0 = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check 'dry run exits 0' ($r.Code -eq 0) $r.Text
  Check 'dry run offers the patch 1.6.5 first' ($r.Text -match '1\) patch -> v1\.6\.5') ''
  Check 'dry run offers no "again" option' ($r.Text -notmatch 'again \(release commit') ''
  Check 'dry run creates no tag' (-not (G $work @('tag', '--list', 'v1.6.5'))) ''
  Check 'dry run leaves HEAD and VERSION' ((G $work @('rev-parse', 'HEAD')) -eq $head0 -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.4') ''

  # 1b. With no release tag yet, a version that is not newer than VERSION needs a confirmation (1.6.4 shipped untagged).
  $r = Invoke-Release @('1', '1.6.4', 'n', '1.6.3', 'n')
  Check 'VERSION itself needs a confirmation' ($r.Text -match 'v1\.6\.4 is not newer than VERSION 1\.6\.4') $r.Text
  Check 'a lower version needs one too' ($r.Text -match 'v1\.6\.3 is not newer than VERSION 1\.6\.4') $r.Text
  Check 'declining both makes no tag' ($r.Code -eq 1 -and -not (G $work @('tag', '--list', 'v*'))) ''

  # 2. First release: a normal patch bump from the seeded 1.6.4.
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'first release exits 0' ($r.Code -eq 0) $r.Text
  Check 'first release commits VERSION 1.6.5' ((G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.5' -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.5') ''
  Check 'v1.6.5 is annotated' ((G $work @('cat-file', '-t', 'v1.6.5')) -eq 'tag') ''
  Check 'v1.6.5 tags the release commit' ((G $work @('rev-parse', 'v1.6.5^{commit}')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin has v1.6.5' ([bool](G $origin @('tag', '--list', 'v1.6.5'))) ''
  Check 'tag message' ((G $work @('tag', '-l', '--format=%(contents:subject)', 'v1.6.5')) -eq 'D2BS v1.6.5') ''

  # 3. Patch release with the review gate's diff: VERSION commit, tag and push.
  $r = Invoke-Release @('1', '', 'y', 'd', 'y')
  Check 'patch release exits 0' ($r.Code -eq 0) $r.Text
  Check 'patch release offered 1.6.6 first' ($r.Text -match '1\) patch -> v1\.6\.6') ''
  Check 'VERSION is 1.6.6' ((Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw) -eq "1.6.6`n") ''
  Check 'release commit subject' ((G $work @('log', '-1', '--format=%s')) -eq 'chore(release): v1.6.6') ''
  Check 'release commit holds only VERSION' ((G $work @('show', '--name-only', '--format=', 'HEAD')) -eq 'VERSION') ''
  Check 'v1.6.6 tags the release commit' ((G $work @('rev-parse', 'v1.6.6^{commit}')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin master moved' ((G $origin @('rev-parse', 'master')) -eq (G $work @('rev-parse', 'HEAD'))) ''
  Check 'origin has v1.6.6' ([bool](G $origin @('tag', '--list', 'v1.6.6'))) ''
  Check 'the d answer showed the diff' ($r.Text -match '\+1\.6\.6') ''

  # 4. A taken or malformed version is asked again; abort at review puts VERSION back.
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

  # 5. Preconditions: a tracked change stops a real run; a dry run only warns.
  Add-Content -LiteralPath (Join-Path $work 'D2BS.h') -Value '// sandbox edit'
  $r = Invoke-Release @('1', '', 'y')
  Check 'tracked change stops the release' ($r.Code -eq 1 -and $r.Text -match 'tracked change') $r.Text
  $r = Invoke-Release @('1', '', 'y') @('-DryRun')
  Check 'dry run only warns about it' ($r.Code -eq 0 -and $r.Text -match 'tracked change') $r.Text
  G $work @('checkout', '--quiet', '--', 'D2BS.h') | Out-Null

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

  # 9. Input that ends at the review prompt puts VERSION back.
  $r = Invoke-Release @('1', '', 'y')
  Check 'end of input at review stops' ($r.Code -eq 1 -and $r.Text -match 'no more input') $r.Text
  Check 'and puts VERSION back' ($r.Text -match 'VERSION put back' -and -not (G $work @('status', '--porcelain'))) $r.Text

  # 10. A rejected push prints undo commands that work as printed.
  $hook = Join-Path $origin 'hooks' 'pre-receive'
  [IO.File]::WriteAllText($hook, "#!/bin/sh`necho sandbox rejects pushes`nexit 1`n")
  $head = G $work @('rev-parse', 'HEAD')
  $r = Invoke-Release @('1', '', 'y', 'y')
  Check 'rejected push exits 1' ($r.Code -eq 1 -and $r.Text -match 'push failed') $r.Text
  $undo = @($r.Text -split "`n" | Where-Object { $_ -match '^  git ' } | ForEach-Object { $_.Trim() })
  Check 'undo commands are separate lines' ($undo.Count -eq 3 -and $undo[0] -eq 'git reset --keep HEAD~1' -and $undo[1] -eq 'git tag -d v1.6.7') ($undo -join ' | ')
  Check 'origin got nothing' (-not (G $origin @('tag', '--list', 'v1.6.7')) -and (G $origin @('rev-parse', 'master')) -eq $head) ''
  # An edit to another tracked file, as if made during the prompts, must survive the printed undo.
  Add-Content -LiteralPath (Join-Path $work 'D2BS.h') -Value '// edit made during the prompts'
  foreach ($u in $undo | Select-Object -First 2) { G $work @($u.Substring(4) -split ' ') | Out-Null }
  Check 'the undo restores the release state' ((G $work @('rev-parse', 'HEAD')) -eq $head -and -not (G $work @('tag', '--list', 'v1.6.7')) -and (Get-Content -LiteralPath (Join-Path $work 'VERSION') -Raw).Trim() -eq '1.6.6') ''
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
