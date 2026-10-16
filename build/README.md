# Building D2BS.dll

D2BS links SpiderMonkey from `dependencies/libs/release/js_static.zip`. That library was built with Visual Studio
2010, and it refuses every newer linker (`/FAILIFMISMATCH:_MSC_VER=1600`). The DLL therefore needs the Visual C++ 2010
SP1 (v100) toolset. The scripts here use a copy of that toolset in a plain folder: nothing is installed.

## On GitHub

`.github/workflows/build.yml` builds on every pull request, every push to `master`, a twice-weekly schedule, and on
demand (Run workflow).

- **Every build:** the run's artifact holds `D2BS.dll`, `D2BS.pdb` and `build.log`. It is named `D2BS-<commit>` and
  kept for 30 days; a tag build's is named `D2BS-<tag>` and kept for 90.
- **Tags `v<major>.<minor>.<patch>`:** the build is a release, and the workflow creates a draft GitHub Release with
  the DLL and PDB. You publish the draft by hand (see Releasing).
- **The toolset:** on its first run, the workflow downloads the two Microsoft packages in `toolset.json` (about
  725 MB), checks their SHA-256, and unpacks them. It keeps the result in the Actions cache.
  - Later runs restore the cache instead of downloading again.
  - GitHub drops a cache entry after 7 days without use; the scheduled runs keep it in use.
  - A change to `toolset.json` or `Get-Toolset.ps1` starts a fresh entry.

## Locally

```powershell
pwsh -File build\Get-Toolset.ps1 -Out C:\d2bs-toolset                    # once; -PackageDir <folder> uses downloaded files
pwsh -File build\Build-D2BS.ps1 -Toolset C:\d2bs-toolset                 # writes build\out\D2BS.dll
```

You need PowerShell 7, 7-Zip and a Windows 10 or later SDK (for `rc.exe`). Set `D2BS_TOOLSET` to skip `-Toolset`.
Everything the build writes goes to `build\out` (ignored by git), or to the folder you pass with `-Out`. The toolset
brings its own copy of the Visual C++ 2010 runtime, so the machine doesn't need it installed.

## Versions

`VERSION` holds the latest release (`major.minor.patch`). The release script bumps it. The build stamps it, with the
commit, into the startup log line, `version()`, the on-screen logo, and the DLL's version information. The product
version string in the file's Properties carries the same text as the log line. The last file-version number is 0
only for a release.

| Build | Shows | File version |
| --- | --- | --- |
| Release (tag `v1.6.5`) | `1.6.5U (7c036ba)` | 1.6.5.0 |
| CI build after that release | `1.6.5U+dev (7c036ba)` | 1.6.5.<run number> |
| Local script build | `1.6.5U+dev (7c036ba)` | 1.6.5.65535, or the `-BuildNumber` you pass |
| Built from uncommitted changes | `... (7c036ba+dirty)` | |
| Visual Studio, without the scripts | `local build` | 0.0.0.0 |

The version block is in `version.rc2`, not `version.rc`. Visual Studio's resource editor rewrites `version.rc` and
would turn its macros into fixed numbers. As a backstop, the build fails if the DLL's version doesn't match.

## Releasing

```powershell
pwsh -File build\Release-D2BS.ps1            # -DryRun rehearses it and changes nothing
```

On an up-to-date `master` with no uncommitted changes, the script:

1. Checks the preconditions: on `master`, no tracked changes, not behind `origin/master`, and the last CI build of
   `master`.
2. Offers the version. `VERSION` as it stands if that version has no tag yet (the first release), otherwise the
   next patch, minor or major. You can type another.
3. Writes `VERSION` and shows the change for review.
4. Commits it as `chore(release): v1.6.5`.
5. Creates the annotated tag `v1.6.5`.
6. Pushes `master` and the tag in one atomic push.

The tag build creates a draft release. Check the DLL, then publish the draft:
`gh release edit v1.6.5 --repo blizzhackers/d2bs --draft=false --latest`.

If the tag build fails, a download problem only needs a re-run from the Actions page. If the code is at fault:

1. Delete the tag on GitHub and locally: `git push origin :refs/tags/v1.6.5`, then `git tag -d v1.6.5`.
2. Fix `master`.
3. Run the script again. It offers the same version again.
