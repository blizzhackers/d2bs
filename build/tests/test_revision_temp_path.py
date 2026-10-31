"""Compile/run the real Win32 path helper with v100, without loading D2BS or the game.

python build/tests/test_revision_temp_path.py --toolset C:/d2bs-toolset
All generated files and child TEMP directories stay under build/out/revision-tests.
"""
import argparse
import concurrent.futures
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--toolset', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    toolset = args.toolset.resolve()
    out = root / 'build/out/revision-tests'
    out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ)
    env['PATH'] = str(toolset / 'vc/bin') + ';' + str(toolset / 'ide') + ';' + env['PATH']
    env['INCLUDE'] = str(toolset / 'vc/include') + ';' + str(toolset / 'sdk/include')
    env['LIB'] = str(toolset / 'vc/lib') + ';' + str(toolset / 'sdk/lib')
    exe = out / 'RevisionTempPathTest.exe'
    command = [str(toolset / 'vc/bin/cl.exe'), '/nologo', '/W4', '/WX', '/wd4127', '/O2', '/MD',
               '/DWIN32_LEAN_AND_MEAN', '/D_WIN32_WINNT=0x0501', '/I' + str(root),
               '/Fe' + str(exe), '/Fo' + str(out) + os.sep,
               str(root / 'RevisionTempPath.cpp'), str(root / 'build/tests/RevisionTempPathTest.cpp')]
    result = subprocess.run(command, cwd=out, env=env, capture_output=True, text=True, timeout=60)
    (out / 'compile.log').write_text(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)

    with tempfile.TemporaryDirectory(prefix='clients-', dir=out) as scratch:
        env['TMP'] = env['TEMP'] = scratch
        subprocess.run([str(exe), 'self'], env=env, check=True, timeout=15)
        # An existing, usable root (CreateDirectoryA takes up to 248 characters) with no room for the per-process
        # folder: a helper that fell back to the root itself would pass it back. Windows replaces a root longer
        # than MAX_PATH with USERPROFILE, so such a root would not test the helper.
        near_limit = scratch + '\\' + 'x' * (244 - len(scratch))
        os.mkdir(near_limit)
        try:
            for invalid in (str(Path(scratch) / 'missing' / 'nested'), near_limit):
                bad_env = dict(env, TMP=invalid, TEMP=invalid)
                subprocess.run([str(exe), 'invalid-base'], env=bad_env, check=True, timeout=15)
        finally:
            os.rmdir(near_limit)
        print('PASS: missing and near-MAX_PATH temporary roots fail without a shared-path fallback')
        for mode in ('shared', 'isolated'):
            clients = []
            try:
                for _ in range(16):
                    clients.append(subprocess.Popen([str(exe), mode], env=env, stdin=subprocess.PIPE,
                                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True))
                pool = concurrent.futures.ThreadPoolExecutor(max_workers=len(clients))
                try:
                    reads = [pool.submit(p.stdout.readline) for p in clients]
                    lines = [f.result(timeout=15).strip() for f in reads]
                finally:
                    # On a timeout, terminate the children before waiting on blocked reader threads.
                    if any(not f.done() for f in reads):
                        for p in clients:
                            if p.poll() is None:
                                p.kill()
                    pool.shutdown(wait=True)
                ready = [line[6:] for line in lines if line.startswith('READY ')]
                expected = 1 if mode == 'shared' else len(clients)
                assert len(ready) == expected, (mode, lines)
                assert len(set(ready)) == expected, (mode, lines)
                for p, line in zip(clients, lines):
                    if line.startswith('READY '):
                        p.stdin.write('\n')
                        p.stdin.flush()
                        assert p.wait(timeout=15) == 0, p.stderr.read()
                    else:
                        assert line == 'BLOCKED 32', line  # ERROR_SHARING_VIOLATION
                        assert p.wait(timeout=15) == 2, p.stderr.read()
                print(f'PASS: {mode}: {expected}/16 concurrent extractions succeeded; contents and cleanup checked')
            finally:
                for p in clients:
                    if p.poll() is None:
                        p.kill()
                    p.wait(timeout=15)
                    for stream in (p.stdin, p.stdout, p.stderr):
                        stream.close()

        ended = []
        for _ in range(3):
            client = subprocess.Popen([str(exe), 'init'], env=env)
            assert client.wait(timeout=15) == 0
            ended.append(client.pid)
        prefix = str(Path(scratch) / 'd2bs-revision-')
        stale = Path(f'{prefix}{ended[0]}')
        holding = Path(f'{prefix}{ended[1]}')
        live = Path(f'{prefix}{os.getpid()}')
        odd = [Path(prefix + name) for name in ('', 'abc', '12x', '+1')]
        for folder in [stale, holding, live] + odd:
            folder.mkdir()
        (holding / 'CheckRevision.dll').write_bytes(b'x')
        not_folder = Path(f'{prefix}{ended[2]}')
        not_folder.write_bytes(b'x')
        subprocess.run([str(exe), 'init'], env=env, check=True, timeout=15)
        kept = [holding, live, not_folder] + odd
        assert not stale.exists() and all(p.exists() for p in kept), [p.name for p in [stale] + kept if p.exists()]
        (holding / 'CheckRevision.dll').unlink()
        not_folder.unlink()
        for folder in [holding, live] + odd:
            folder.rmdir()
        print('PASS: startup removes empty folders of ended processes only')
        assert not list(Path(scratch).iterdir()), 'test clients left temporary files or directories'


if __name__ == '__main__':
    main()
