# Building D2BS.dll

D2BS links SpiderMonkey from `dependencies/libs/release/js_static.zip`. That library was built with Visual Studio
2010, and it refuses every newer linker (`/FAILIFMISMATCH:_MSC_VER=1600`). The DLL therefore needs the Visual C++ 2010
SP1 (v100) toolset. The scripts here use a copy of that toolset in a plain folder: nothing is installed.

## On GitHub

`.github/workflows/build.yml` builds on every pull request, every push to `master`, three times a week on a schedule,
and on demand (Run workflow).

- **Every build:** the run's artifact holds `D2BS.dll`, `D2BS.pdb` and `build.log` (a release tag's artifact also
  holds `release-notes.md`). It is named `D2BS-<commit>` and kept for 30 days. A pull request builds its own head
  commit, so that commit is the one in the name and in the DLL.
  A build that fails while compiling, linking or checking the DLL's version uploads its whole `build.log` alone, as
  `D2BS-build-log-<commit>` (or `-<tag>`); the job log shows the failing tool's first errors and last lines. A failure
  before that (a tag whose release notes fail their checks, fetching the toolset, a missing toolset, a tag that
  doesn't match `VERSION`) shows its cause in the job log.
- **Release tags (`v<major>.<minor>.<patch>`, e.g. `v1.6.5`) and release candidates' (`v1.6.5-rc.1`):** the build is
  a release, and the workflow creates a GitHub Release with the DLL and PDB. A final's is a draft, which you publish
  by hand (see Releasing); a release candidate's is published at once as a pre-release, never marked Latest. Its notes
  are the version's section of `CHANGELOG.md`, followed by GitHub's generated list of pull requests since the release
  tag the section's compare link starts from (when the link starts from a commit, or that tag no longer exists, GitHub
  picks the start). A tag fails before the build when its version has no section there, the section has no entries,
  or an HTML comment or code fence in the section never closes. The artifact is named `D2BS-<tag>` and kept for 90
  days. Other tags don't start a build.
- **The toolset:** on its first run, the workflow downloads the two Microsoft packages in `toolset.json` (about
  725 MB), checks their SHA-256, and unpacks them. It keeps the result in the Actions cache.
  - Later runs restore the cache instead of downloading again.
  - GitHub drops a cache entry after 7 days without use; the scheduled runs keep it in use.
  - A change to `toolset.json` or `Get-Toolset.ps1` starts a fresh entry.
  - A pull request's entry is visible only to that pull request, so the first build on `master` downloads once more.

## Locally

```powershell
pwsh -File build\Get-Toolset.ps1 -Out C:\d2bs-toolset                    # once; -PackageDir <folder> uses downloaded files
pwsh -File build\Build-D2BS.ps1 -Toolset C:\d2bs-toolset                 # writes build\out\D2BS.dll
```

You need PowerShell 7.2 or later, 7-Zip and a Windows 10 or later SDK (for `rc.exe`). Set `D2BS_TOOLSET` to skip
`-Toolset`. Everything the build writes goes to `build\out` (ignored by git), or to the folder you pass with `-Out`.
The toolset brings its own copy of the Visual C++ 2010 runtime, so the machine doesn't need it installed.
`Get-Toolset.ps1` replaces its `-Out` folder only once the new toolset passes its checks, and it refuses a folder
that holds anything other than an earlier toolset. The build refuses a toolset made from other packages or by an
older `Get-Toolset.ps1`: run `Get-Toolset.ps1` again. Whoever changes what `Get-Toolset.ps1` puts in the toolset
bumps `recipe` in `toolset.json`; a package change is caught by its SHA-256 alone.

## Testing revision temporary directories

```powershell
python build\tests\test_revision_temp_path.py --toolset C:\d2bs-toolset
```

This compiles the Win32 path helper with the same v100 toolset and runs 16 concurrent clients against shared and
isolated revision filenames, plus buffer-limit, path-failure, cleanup and startup-cleanup checks. Run it after
changing `RevisionTempPath.cpp`. It needs no game or Battle.net connection, so it can't show the game's own version
check; that takes several games starting Battle.net at once. Generated files stay in `build\out\revision-tests`.

## Versions

`VERSION` holds the latest release or release candidate (`major.minor.patch`, or `major.minor.patch-rc.N`), and the
release script bumps it. It starts at `1.6.4`, the last version shipped (as 1.6.4U). The build stamps it, with the
commit, into the startup log line, `version()` and the DLL's version information; the on-screen logo shows it without
the commit. The product version string in the file's Properties carries the same text as the log line. The last
file-version number is 0 only for a final release.

| Build | Log line and `version()` | Logo | File version |
| --- | --- | --- | --- |
| Release (tag `v1.6.5`) | `1.6.5U (7c036ba)` | `D2BS 1.6.5U` | 1.6.5.0 |
| CI build after that release | `1.6.5U+dev (7c036ba)` | `D2BS 1.6.5U+dev` | 1.6.5.<run number> |
| Release candidate (tag `v1.7.0-rc.2`) | `1.7.0U-rc.2 (7c036ba)` | `D2BS 1.7.0U-rc.2` | 1.7.0.<run number> |
| CI build after that release candidate | `1.7.0U-rc.2+dev (7c036ba)` | `D2BS 1.7.0U-rc.2+dev` | 1.7.0.<run number> |
| Local script build | `1.6.5U+dev (7c036ba)` | `D2BS 1.6.5U+dev` | 1.6.5.65535, or the `-BuildNumber` you pass |
| Built with uncommitted changes to tracked files | `... (7c036ba+dirty)` | | |
| Visual Studio, without the scripts | `local build` | `D2BS local build` | 0.0.0.0 |

The version block is in `version.rc2`, not `version.rc`. Visual Studio's resource editor rewrites `version.rc` and
would turn its macros into fixed numbers. As a backstop, the build fails if the DLL's version doesn't match.

## Releasing

```powershell
pwsh -File build\Release-D2BS.ps1            # -DryRun rehearses it and changes nothing
```

The release notes come from `CHANGELOG.md`, in the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format.
A change that script authors or bot operators would notice adds its entry under `## [Unreleased]`, in the same pull
request: one line per entry, opening with a short bold lead, because GitHub shows every line break of a release's
notes. `pwsh -File build\Get-ReleaseNotes.ps1 -Version 1.6.5` prints the notes a version's release opens with.

### Release candidates

A version can go out to testers first as release candidates: `v1.7.0-rc.1`, then `v1.7.0-rc.2` with the fixes for
what they found, and so on, each a DLL that names itself (`1.7.0U-rc.2 (7c036ba)`), until one holds up and becomes
`v1.7.0`. Each release candidate dates `[Unreleased]` as its own section (`## [1.7.0-rc.2] - <date>`), so its notes
list just what changed since the one before. The final merges those sections and `[Unreleased]` into one
`## [1.7.0] - <date>`: each group (Added, Changed, Fixed, ...) once, with the oldest release candidate's entries first.
At its review, drop the entries for problems that only a release candidate had, which users of the last final
release never met. A version can also skip release candidates and go straight to its final.

### The release script

On an up-to-date `master` with no uncommitted changes, the script:

1. Checks the preconditions: on `master`, no tracked changes, not behind `origin/master`, a committed `CHANGELOG.md`
   with an `## [Unreleased]` heading, and the last CI build of `master`.
2. Counts the entries under `[Unreleased]` and each release candidate's section per group, and offers the next
   version: after a final release, the first release candidate of the next patch, minor or major version (e.g.
   `1.6.5-rc.1` after `1.6.4`), or that version's final; after a release candidate, the next one (`1.6.5-rc.2`) or the
   final (`1.6.5`). The default is the same version again when that is offered; otherwise the patch release
   candidate after a final, and the next release candidate after one, or the final when nothing new is under
   `[Unreleased]`. You can type another version; one that isn't newer than `VERSION` or the latest release tag
   (release candidates included) needs a confirmation (except when it offers the same version again, below). A
   release candidate needs entries under `[Unreleased]`; a final needs them there or in its release candidates'
   sections. A final warns about commits made since the last release candidate, which no tester has run.
3. Shows the plan with the release notes. Notes the tag build would refuse (a missing section, no entries, or an
   HTML comment or code fence that never closes) stop it there instead. After you confirm (it stops if `VERSION`,
   `CHANGELOG.md` or the commit changed during the prompts), it writes `VERSION`, updates `CHANGELOG.md` as above
   (under a new, empty `[Unreleased]`), points the links at the bottom at the new tag, and shows the change for
   review. You can edit `CHANGELOG.md` before answering the review; `d` shows the diff as it is then, and `y` takes
   the files as they are, as long as `VERSION` still names the version and the notes still pass those checks.
   Leaving the review any other way puts both files back, after copying an edited `CHANGELOG.md` to your temp folder
   (the script prints where).
4. Commits both files as `chore(release): v1.6.5`, unless a commit was made during the review, which stops it.
5. Creates the annotated tag `v1.6.5`.
6. Pushes `master` and the tag in one atomic push.

A release candidate's tag build publishes its pre-release, ready to hand to testers. A final's tag build creates a
draft release: try the DLL, then publish the draft:
`gh release edit v1.6.5 --repo blizzhackers/d2bs --draft=false --latest`.

If the tag build fails on a download, re-run it from its Actions page. If the code or the DLL is at fault (for a
release candidate, only while no tester has it; after that, cut the next release candidate, so a name always means
one DLL):

1. Delete the release if there is one: `gh release delete v1.6.5 --repo blizzhackers/d2bs --yes`.
2. Delete the tag on GitHub and locally: `git push origin :refs/tags/v1.6.5`, then `git tag -d v1.6.5`.
3. Fix `master`. A fix that needs a changelog entry goes under `## [1.6.5]`, the section the release posts, not
   under `[Unreleased]`.
4. Run the script again. It offers the same version again, for the fixed `master`.

Run `pwsh -File build\tests\Test-ReleaseScript.ps1` after changing `Release-D2BS.ps1`, `Changelog.psm1` or
`Get-ReleaseNotes.ps1`. It releases into a throwaway sandbox (a bare clone of this repo without its tags as `origin`,
with your working copies of the scripts and a `CHANGELOG.md` of its own) through every path above that needs no
GitHub, including a rejected push, a re-release and a release candidate cycle up to its final, and nothing reaches
this repo or GitHub.
The check of `master`'s CI build and the watch of the tag build need GitHub, so the test can't run them. Its exit
code is the number of failed checks, or 1 if a git step in the sandbox itself fails.
