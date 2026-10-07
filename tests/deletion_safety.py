"""Isolated cleanup tests. Fixtures stay under tests/output/deletion-safety.

Windows: python tests/deletion_safety.py [--native path/to/deletion_safety.exe]
POSIX:   python3 tests/deletion_safety.py --native path/to/deletion_safety
No production build, upload, desktop control, or repository cleanup is executed.
"""
import argparse
import os
from pathlib import Path
import subprocess
import uuid
import re
import shutil

REPO = Path(__file__).resolve().parent.parent
OUTPUT = REPO / "tests" / "output" / "deletion-safety"
OUTPUT.mkdir(parents=True, exist_ok=True)
FIXTURE = OUTPUT / str(uuid.uuid4())
FIXTURE.mkdir()


def run(args, ok=True, **kwargs):
    result = subprocess.run(args, cwd=FIXTURE, capture_output=True, text=True, **kwargs)
    assert (result.returncode == 0) == ok, (args, result.returncode, result.stdout, result.stderr)
    return result


def audit_batch_cleanup():
    # Keep the original high-risk batch idioms from returning. Vendored build
    # scripts are outside this audit; the runtime tests verify the helper itself.
    tracked = subprocess.check_output(["git", "-C", str(REPO), "ls-files", "*.bat"], text=True)
    for name in tracked.splitlines():
        if name.startswith("shared/"):
            continue
        lines = (REPO / name).read_text().splitlines()
        for i, line in enumerate(lines):
            if line.lstrip().lower().startswith(("rem ", ":")):
                continue
            assert not re.search(r"(?i)^\s*(?:rd|rmdir|deltree)\b|\bdel\s+.*[%*?]", line), (name, i+1, line)
            if "SafeRemove.ps1" in line:
                assert lines[i+1].strip().lower() == "if errorlevel 1 exit /b 1", (name, i+1)
    print("First-party batch cleanup audit passed")


def windows_tests():
    helper = REPO / "shared/win/utils/SafeRemove.ps1"
    def remove(root, child, *flags, ok=True):
        return run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                    str(helper), "-Root", str(root), "-RelativePath", child, *flags], ok=ok)
    project = FIXTURE / "project with spaces"
    project.mkdir()
    (project / "media/interface/nested").mkdir(parents=True)
    (project / "media/interface/nested/a.rttex").write_text("packed")
    (project / "media/interface/keep.png").write_text("keep")
    (FIXTURE / "keep.rttex").write_text("wrong working directory sentinel")
    (project / "keep.txt").write_text("keep")
    remove(project, "media/interface/*.rttex", "-Recurse", "-FilesOnly")
    assert not (project / "media/interface/nested/a.rttex").exists()
    assert (project / "media/interface/keep.png").exists()
    assert (FIXTURE / "keep.rttex").exists()
    remove(project, "media/interface", "-Recurse")
    assert not (project / "media/interface").exists()
    remove(project, "missing", "-Recurse")
    remove(project, "media", "-FilesOnly", ok=False)
    remove(project, "keep.txt", "-FilesOnly")
    assert not (project / "keep.txt").exists()
    for root in ["", " ", ".", "C:", "C:\\", "C:relative"]:
        remove(root, "proton-nonexistent-" + uuid.uuid4().hex, "-Recurse", ok=False)
    for child in ["", ".", "..", "../outside", "media/../../outside", "media/*/x",
                  "media/child.", "media/child ", "media:stream", "media/*"]:
        remove(project, child, "-Recurse", ok=False)
    outside = FIXTURE / "outside"
    outside.mkdir()
    (outside / "sentinel.txt").write_text("keep")
    # Native PowerShell junction creation needs no administrator rights.
    junction = project / "junction"
    script = "New-Item -ItemType Junction -Path '{}' -Target '{}' | Out-Null".format(
        str(junction).replace("'", "''"), str(outside).replace("'", "''"))
    run(["powershell.exe", "-NoProfile", "-Command", script])
    remove(project, "junction", "-Recurse", ok=False)
    remove(project, "junction/sentinel.txt", "-FilesOnly", ok=False)
    assert (outside / "sentinel.txt").exists()
    # Execute real batch entry points in a copy of the minimal project layout,
    # from a deliberately wrong cwd; keep filenames with spaces in the fixture.
    tool_dir = FIXTURE / "shared/win/utils"
    tool_dir.mkdir(parents=True)
    shutil.copyfile(helper, tool_dir / "SafeRemove.ps1")
    for app in ["ArduboySim", "RTBareBones", "RTLooneyLadders", "RTShader", "RTSimpleApp"]:
        media = FIXTURE / app / "media"
        media.mkdir(parents=True)
        shutil.copyfile(REPO / app / "media/delete_all_rttex_files.bat", media / "delete_all_rttex_files.bat")
        (media / "file with spaces.rttex").write_text("packed")
        (media / "keep.png").write_text("keep")
        run(["cmd.exe", "/d", "/c", str(media / "delete_all_rttex_files.bat")])
        assert not (media / "file with spaces.rttex").exists()
        assert (media / "keep.png").exists() and (FIXTURE / "keep.rttex").exists()
    # Regenerating Android assets must remove stale directories as well as files.
    java = FIXTURE / "shared/android/v3_src"
    java.mkdir(parents=True)
    (java / "SharedActivity.java").write_text("// fixture")
    for app in ["RTBareBones", "RTShader", "RTSimpleApp"]:
        project = FIXTURE / app
        gradle = project / "AndroidGradle"
        (gradle / "app/src/main/java/com/rtsoft/RTAndroidApp").mkdir(parents=True)
        assets = gradle / "app/src/main/assets"
        (assets / "obsolete/nested").mkdir(parents=True)
        (assets / "obsolete/nested/old.dat").write_text("stale")
        for folder in ["interface", "audio", "game"]:
            source = project / "bin" / folder
            source.mkdir(parents=True)
            (source / "current.dat").write_text("current")
        (project / "bin/save.dat").write_text("keep")
        script = gradle / "PrepareResources.bat"
        shutil.copyfile(REPO / app / "AndroidGradle/PrepareResources.bat", script)
        run(["cmd.exe", "/d", "/c", str(script)])
        assert not (assets / "obsolete").exists()
        assert all((assets / folder / "current.dat").exists() for folder in ["interface", "audio", "game"])
        assert (project / "bin/save.dat").read_text() == "keep"
    print("Windows cleanup and batch entry-point tests passed")


def posix_tests():
    helper = REPO / "shared/linux/safe_paths.sh"
    project = FIXTURE / "project with spaces"
    project.mkdir()
    (project / "build/nested").mkdir(parents=True)
    (project / "build/nested/generated.txt").write_text("generated")
    (FIXTURE / "keep.txt").write_text("keep")
    command = '. "$1"; proton_remove_tree "$2" "$3"'
    def remove(root, child, ok=True):
        return run(["sh", "-c", command, "sh", str(helper), str(root), child], ok=ok)
    remove(project, "build")
    assert not (project / "build").exists() and (FIXTURE / "keep.txt").exists()
    for root in ["", ".", "/", "//"]:
        remove(root, "proton-nonexistent-" + uuid.uuid4().hex, ok=False)
    for child in ["", ".", "..", "../outside", "/tmp", "build/../../outside", "*", "build/*", "build//x"]:
        remove(project, child, ok=False)
    (project / "link").symlink_to(FIXTURE, target_is_directory=True)
    remove(project, "link", ok=False)
    remove(project, "link/keep.txt", ok=False)
    remote = REPO / "shared/linux/clean_web_loader.sh"
    home = FIXTURE / "remote home"
    home.mkdir()
    loader = home / "www/web/rtbarebones/WebLoaderData"
    loader.mkdir(parents=True)
    (loader / "file.txt").write_text("generated")
    (home / "www/web/rtbarebones/keep.txt").write_text("keep")
    env = dict(os.environ, HOME=str(home))
    run(["sh", str(remote), "rtbarebones"], env=env)
    assert not loader.exists()
    assert (home / "www/web/rtbarebones/keep.txt").exists()
    for bad in ["", "/", "relative"]:
        run(["sh", str(remote), "rtbarebones"], ok=False, env=dict(env, HOME=bad))
    run(["sh", str(remote), "../bad"], ok=False, env=env)
    # Refuse missing explicit project arguments before any external build tools.
    for name in ["androidSyncAssets.sh", "prepareAndroid.sh", "update_media.sh"]:
        run(["bash", str(REPO / "RTPack/linux" / name)], ok=False)
    # Run the real positive asset-sync paths against fixtures. Packing itself is
    # stubbed; rsync and cleanup are real, including stale nested directories.
    tool_repo = FIXTURE / "tool repo"
    scripts = tool_repo / "RTPack/linux"
    scripts.mkdir(parents=True)
    shared = tool_repo / "shared/linux"
    shared.mkdir(parents=True)
    shutil.copyfile(helper, shared / "safe_paths.sh")
    for name in ["update_media.sh", "androidSyncAssets.sh"]:
        shutil.copyfile(REPO / "RTPack/linux" / name, scripts / name)
    pack = tool_repo / "RTPack/bin/RTPack"
    pack.parent.mkdir()
    pack.write_text("#!/bin/sh\nexit 0\n")
    pack.chmod(0o755)
    app = tool_repo / "FixtureApp"
    media = app / "media"
    for folder in ["interface", "audio", "game"]:
        (media / folder).mkdir(parents=True)
        (media / folder / "current.dat").write_text("current")
        stale = app / "bin" / folder / "stale/nested"
        stale.mkdir(parents=True)
        (stale / "old.dat").write_text("stale")
    (media / "exclude.txt").write_text(".txt\n")
    (media / "game_exclude.txt").write_text(".txt\n")
    (media / "interface/font_old.rttex").write_text("temporary")
    (media / "icon.rttex").write_text("temporary")
    (media / "default.rttex").write_text("temporary")
    (app / "bin/save.dat").write_text("keep")
    run(["bash", str(scripts / "update_media.sh"), str(media)])
    for folder in ["interface", "audio", "game"]:
        assert not (app / "bin" / folder / "stale").exists()
        assert (app / "bin" / folder / "current.dat").exists()
    assert not (media / "interface/font_old.rttex").exists()
    assert not (media / "icon.rttex").exists() and not (media / "default.rttex").exists()
    assert (app / "bin/save.dat").read_text() == "keep"
    android = app / "android"
    (android / "assets/obsolete/nested").mkdir(parents=True)
    (android / "assets/obsolete/nested/old.dat").write_text("stale")
    (android / "AndroidManifest.xml").write_text("<manifest/>")
    run(["bash", str(scripts / "androidSyncAssets.sh"), str(android)])
    assert not (android / "assets/obsolete").exists()
    assert not (android / "assets/save.dat").exists()
    assert (android / "assets/interface/current.dat").exists()
    print("Media and Android stale-directory cleanup tests passed")
    print("POSIX cleanup, remote cleanup and argument tests passed")


def gradle_tests(wrapper):
    # Exercise the exact clean task without loading Android plugins, signing
    # properties, or the production app's PrepareResources configuration hook.
    wrapper = str(wrapper.resolve())
    for app in ["RTBareBones", "RTShader", "RTSimpleApp"]:
        source = (REPO / app / "AndroidGradle/build.gradle").read_text()
        task = source[source.index("task clean(type: Delete)"):]
        for redirected in [False, True]:
            project = FIXTURE / (app + ("-redirected" if redirected else "-normal"))
            project.mkdir()
            (project / "settings.gradle").write_text("rootProject.name = 'cleanup-fixture'\n")
            extra = "\nrootProject.buildDir = file('../outside-gradle')\n" if redirected else ""
            (project / "build.gradle").write_text(task + extra)
            (project / "build/nested").mkdir(parents=True)
            (project / "build/nested/stale.dat").write_text("stale")
            (project / "keep.dat").write_text("keep")
            outside = FIXTURE / "outside-gradle"
            outside.mkdir(exist_ok=True)
            (outside / "keep.dat").write_text("keep")
            command = [wrapper, "-p", str(project), "--no-daemon", "--console=plain", "clean"]
            if os.name == "nt":
                command = ["cmd.exe", "/d", "/c"] + command
            run(command, ok=not redirected)
            assert (project / "build").exists() == redirected
            assert (project / "keep.dat").read_text() == "keep"
            assert (outside / "keep.dat").read_text() == "keep"
        print(app, "Gradle clean and redirected-root rejection passed", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--native", type=Path, action="append", default=[])
    parser.add_argument("--gradle-wrapper", type=Path, help="Optional Gradle wrapper; requires a suitable JAVA_HOME")
    args = parser.parse_args()
    audit_batch_cleanup()
    if os.name == "nt":
        windows_tests()
    else:
        posix_tests()
    for index, native in enumerate(args.native):
        native_dir = FIXTURE / ("native-" + str(index))
        native_dir.mkdir()
        print(run([str(native.resolve()), str(native_dir)]).stdout.strip())
    if args.gradle_wrapper:
        gradle_tests(args.gradle_wrapper)
    print("Fixtures retained at", FIXTURE)
