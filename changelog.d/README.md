# Changelog entries

Each file here holds one entry for the next release's notes. A change that script authors or bot operators would
notice adds its file in the same pull request, so two pull requests never edit the same lines of `CHANGELOG.md`.

Name the file `<slug>.<section>.md`:

- `<slug>` names the change in lowercase letters, digits and hyphens, for example `steady-preset-lists`. Two files
  can't share a slug.
- `<section>` is its Keep a Changelog group, in lowercase: `added`, `changed`, `deprecated`, `removed`, `fixed` or
  `security`.

The file holds exactly one line in UTF-8, the entry as `CHANGELOG.md` shows it, opening with a short bold lead:

```markdown
- **Steady preset lists.** `getPresetUnits()` keeps a room's preset list as it is however often it is called.
```

One line, because GitHub shows every line break of a release's notes.

At the next release, `build\Release-D2BS.ps1` adds every entry here to the `[Unreleased]` section of `CHANGELOG.md`,
under its group's heading and in the order the files reached `master`, and its release commit deletes the files. The
order follows `master`'s first-parent history as GitHub's merges leave it; in a shallow clone, the files already there
at its oldest commit go first, by name. Any other file or folder here (this README aside), a file with more than one
line or not in UTF-8, a slug used twice, a file not yet committed or a committed one missing here (deleted, or left
out of a sparse checkout) stops a release of a new version at its plan until it is fixed. Releasing a version again
after its tag was deleted commits nothing, so there they only warn, and a file here already edited or deleted when
the script starts stops it sooner, at its check for tracked changes.

`pwsh -File build\Release-D2BS.ps1 -DryRun` shows the `[Unreleased]` section with these entries in it, changing no
file and making no commit or tag; it still fetches `origin` and its tags unless `-NoFetch` is given as well.
