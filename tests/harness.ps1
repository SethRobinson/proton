# Proton renderer regression harness (Windows).
#
# Launches each app defined in scenarios.psd1 with Proton's -autoscreenshot
# command line parm (see BaseApp::ProcessAutoScreenshot): the app itself writes
# a BMP of its framebuffer at a fixed app-clock time and quits. No desktop
# pixels are ever captured and the window doesn't need to be in the foreground.
# The harness then applies the scenario's ignore-masks and either records the
# shot as a golden or compares it against the golden.
#
#   .\harness.ps1 -Mode golden            # (re)record golden screenshots
#   .\harness.ps1 -Mode test              # capture + compare against goldens
#   .\harness.ps1 -Mode test -App RTDink  # single app
#
# Goldens are GPU/driver specific: only compare on the machine that recorded
# them. Exit code 0 = all pass, 1 = something failed. See tests/README.md.

param(
    [ValidateSet('golden', 'test')] [string]$Mode = 'test',
    [ValidateSet('win', 'html5', 'ios', 'android')] [string]$Target = 'win',
    [string]$App = '*',
    [switch]$ShaderPipe,   # win only: launch apps with -shaderpipeline and diff the shader backend against the same goldens (apps opt in via ShaderReady)
    [switch]$ShowBrowser,  # html5 only: run the browser headed, for debugging
    [switch]$PrepareMac,   # ios only: rsync the tracked tree to the Mac and xcodebuild the sim apps first
    [string]$MacHost = 'seth@studiomac.local', # ios only
    [string]$IosSimDevice = 'iPad Pro (12.9-inch)', # ios only: simulator model to boot/use; goldens are per-model so keep it pinned
    [switch]$IosDevice,    # ios only: run on a real USB/network-paired device instead of the simulator (goldens prefixed iosdev_)
    [string]$IosDeviceId = '', # ios only: devicectl UDID; empty = first paired device
    [string]$MacKeychainPassword = '', # ios only: needed by -PrepareMac -IosDevice to unlock the keychain for codesigning (see agents_secret.md)
    [string]$RepoRoot = (Split-Path $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

Add-Type -TypeDefinition @'
using System;

public static class ProtonHarness
{
    // Per-pixel compare of two same-sized 32bpp BGRA buffers. A pixel counts as
    // different when any color channel differs by more than channelTol (alpha
    // ignored). mask gets 0xFF for differing pixels (one byte per pixel).
    public static long Compare(byte[] a, byte[] b, int channelTol, byte[] mask)
    {
        long diff = 0;
        int pixels = a.Length / 4;
        for (int i = 0; i < pixels; i++)
        {
            int o = i * 4;
            if (Math.Abs(a[o] - b[o]) > channelTol ||
                Math.Abs(a[o + 1] - b[o + 1]) > channelTol ||
                Math.Abs(a[o + 2] - b[o + 2]) > channelTol)
            {
                diff++;
                if (mask != null) mask[i] = 0xFF;
            }
        }
        return diff;
    }
}
'@ -ErrorAction SilentlyContinue

function Get-BitmapBytes([System.Drawing.Bitmap]$bmp)
{
    $rect = New-Object System.Drawing.Rectangle(0, 0, $bmp.Width, $bmp.Height)
    $data = $bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly,
        [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $bytes = New-Object byte[] ($data.Stride * $data.Height)
    [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
    $bmp.UnlockBits($data)
    return $bytes
}

# Convert the app-written BMP to a masked 24bpp PNG (masks cover regions that
# legitimately differ every run: FPS counters, wall-clock animations).
function Convert-ShotToPng([string]$bmpPath, [string]$pngPath, $ignoreRects)
{
    $src = New-Object System.Drawing.Bitmap($bmpPath)
    try
    {
        $rect = New-Object System.Drawing.Rectangle(0, 0, $src.Width, $src.Height)
        $bmp = $src.Clone($rect, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    }
    finally { $src.Dispose() }
    if ($ignoreRects)
    {
        # PowerShell flattens a one-element list of arrays; re-wrap it.
        if ($ignoreRects[0] -isnot [System.Collections.IList]) { $ignoreRects = , $ignoreRects }
        $gfx = [System.Drawing.Graphics]::FromImage($bmp)
        $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::Magenta)
        foreach ($ir in $ignoreRects) { $gfx.FillRectangle($brush, $ir[0], $ir[1], $ir[2], $ir[3]) }
        $brush.Dispose(); $gfx.Dispose()
    }
    $bmp.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}

function Compare-Shots([string]$goldenPath, [string]$capturePath, [int]$channelTol, [string]$diffPath)
{
    $gold = New-Object System.Drawing.Bitmap($goldenPath)
    $test = New-Object System.Drawing.Bitmap($capturePath)
    try
    {
        if ($gold.Width -ne $test.Width -or $gold.Height -ne $test.Height)
        {
            return @{ DiffPct = 100.0; Note = "size mismatch: golden $($gold.Width)x$($gold.Height) vs capture $($test.Width)x$($test.Height)" }
        }
        $mask = New-Object byte[] ($gold.Width * $gold.Height)
        $diff = [ProtonHarness]::Compare((Get-BitmapBytes $gold), (Get-BitmapBytes $test), $channelTol, $mask)
        $pct = 100.0 * $diff / ($gold.Width * $gold.Height)
        if ($diff -gt 0 -and $diffPath)
        {
            # Diff visualization: golden dimmed, differing pixels red
            $vis = New-Object System.Drawing.Bitmap($gold.Width, $gold.Height)
            for ($y = 0; $y -lt $gold.Height; $y++) {
                for ($x = 0; $x -lt $gold.Width; $x++) {
                    if ($mask[$y * $gold.Width + $x]) { $vis.SetPixel($x, $y, [System.Drawing.Color]::Red) }
                    else {
                        $c = $gold.GetPixel($x, $y)
                        $vis.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, [int]($c.R * 0.25), [int]($c.G * 0.25), [int]($c.B * 0.25)))
                    }
                }
            }
            $vis.Save($diffPath, [System.Drawing.Imaging.ImageFormat]::Png)
            $vis.Dispose()
        }
        return @{ DiffPct = $pct; Note = "$diff px differ" }
    }
    finally { $gold.Dispose(); $test.Dispose() }
}

# ---------------------------------------------------------------------------
# html5 target: serve the app's built html5 dir over a local HTTP server,
# launch a (headless) Edge at it with the -autoscreenshot parms in the URL, and
# wait for the app to POST its framebuffer BMP back to us.

function Get-EdgePath
{
    foreach ($p in "${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe",
                   "$env:ProgramFiles\Microsoft\Edge\Application\msedge.exe")
    { if (Test-Path $p) { return $p } }
    throw 'msedge.exe not found; the html5 target needs Edge (or adapt Get-EdgePath for Chrome)'
}

$script:MimeTypes = @{ '.html' = 'text/html'; '.js' = 'text/javascript'; '.wasm' = 'application/wasm'
                       '.data' = 'application/octet-stream'; '.png' = 'image/png'; '.txt' = 'text/plain' }

function Invoke-Html5Capture([string]$pageDir, [string]$pageName, [int]$settleMs, [string]$bmpOutPath, [string]$extraParms = '')
{
    if ($extraParms) { $extraParms = "$extraParms " }
    $port = Get-Random -Minimum 40000 -Maximum 59999
    $listener = New-Object System.Net.HttpListener
    $listener.Prefixes.Add("http://127.0.0.1:$port/")
    $listener.Start()
    $edgeProfile = Join-Path ([IO.Path]::GetTempPath()) "proton_harness_edge"
    $parms = [Uri]::EscapeDataString("$extraParms-autoscreenshot shot.bmp $settleMs")
    $url = "http://127.0.0.1:$port/$pageName`?parms=$parms"
    $edgeArgs = @("--user-data-dir=$edgeProfile", '--no-first-run', '--disable-extensions', '--window-size=1280,960', $url)
    if (-not $ShowBrowser) { $edgeArgs = @('--headless=new') + $edgeArgs }
    $browser = Start-Process -FilePath (Get-EdgePath) -ArgumentList $edgeArgs -PassThru
    $gotShot = $false
    try
    {
        $deadline = (Get-Date).AddMilliseconds($settleMs + 90000)
        while ((Get-Date) -lt $deadline -and -not $gotShot)
        {
            $ctxTask = $listener.GetContextAsync()
            while (-not $ctxTask.Wait(500)) { if ((Get-Date) -ge $deadline) { break } }
            if (-not $ctxTask.IsCompletedSuccessfully) { break }
            $ctx = $ctxTask.Result
            $req = $ctx.Request; $resp = $ctx.Response
            try
            {
                if ($req.HttpMethod -eq 'POST' -and $req.Url.AbsolutePath -like '*autoscreenshot_upload*')
                {
                    $ms = New-Object System.IO.MemoryStream
                    $req.InputStream.CopyTo($ms)
                    $uploadName = $req.QueryString['name']
                    if ($uploadName -like '*.perf.txt')
                    {
                        [IO.File]::WriteAllBytes("$bmpOutPath.perf.txt", $ms.ToArray())
                    }
                    else
                    {
                        # the BMP upload is the "capture finished" signal
                        [IO.File]::WriteAllBytes($bmpOutPath, $ms.ToArray())
                        $gotShot = $true
                    }
                    $resp.StatusCode = 200
                }
                else
                {
                    # static file serving, restricted to the page dir
                    $rel = $req.Url.AbsolutePath.TrimStart('/').Replace('/', '\')
                    $file = [IO.Path]::GetFullPath((Join-Path $pageDir $rel))
                    if ($rel -and -not $file.StartsWith([IO.Path]::GetFullPath($pageDir)) ) { $resp.StatusCode = 403 }
                    elseif ($rel -and (Test-Path $file -PathType Leaf))
                    {
                        $ext = [IO.Path]::GetExtension($file).ToLower()
                        $resp.ContentType = if ($script:MimeTypes.ContainsKey($ext)) { $script:MimeTypes[$ext] } else { 'application/octet-stream' }
                        $bytes = [IO.File]::ReadAllBytes($file)
                        $resp.ContentLength64 = $bytes.Length
                        $resp.OutputStream.Write($bytes, 0, $bytes.Length)
                    }
                    else { $resp.StatusCode = 404 }
                }
            }
            finally { $resp.Close() }
        }
    }
    finally
    {
        $listener.Stop(); $listener.Close()
        # kill the whole browser tree; the profile dir keeps it isolated from the user's own Edge
        & taskkill /PID $browser.Id /T /F 2>$null | Out-Null
    }
    if (-not $gotShot) { throw "no screenshot uploaded within $([int](($settleMs + 90000)/1000))s (try -ShowBrowser to debug; is the html5 build current?)" }
}

# ---------------------------------------------------------------------------
# ios target: builds/runs on the Mac's iOS simulator over ssh. The app is
# launched with the -autoscreenshot parms as launch arguments (see
# shared/iOS/app/main.mm); it writes the BMP to /tmp on the Mac and we scp it
# back. Run with -PrepareMac once after changing engine/app code.

function Invoke-IosCapture([string]$projectPath, [string]$appName, [int]$settleMs, [string]$bmpOutPath)
{
    $remoteBmp = "/tmp/proton_harness_$appName.bmp"
    $timeoutSec = [int](($settleMs + 60000) / 1000)
    # generate a capture script, scp it over, run it - avoids ssh quoting traps
    $lines = @(
        'set -e'
        'APPNAME=$1; REMOTEBMP=$2; SETTLEMS=$3; TIMEOUTSEC=$4; SIMNAME=$5'
        '#pin the simulator model: goldens are per-model, first-available is a lottery'
        'DEV=$(xcrun simctl list devices available | grep -m1 "$SIMNAME (" | grep -oE "[A-F0-9-]{36}")'
        'if [ -z "$DEV" ]; then echo "no available simulator named $SIMNAME"; exit 1; fi'
        'xcrun simctl bootstatus "$DEV" -b >/dev/null' # boots it if needed and waits
        'APP=$(find ~/proton_warncheck/$APPNAME -name "$APPNAME.app" -path "*iphonesimulator*" | head -1)'
        'if [ -z "$APP" ]; then echo "no built .app found - run the harness with -PrepareMac"; exit 1; fi'
        'BUNDLE=$(/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$APP/Info.plist")'
        'xcrun simctl install "$DEV" "$APP"'
        'rm -f "$REMOTEBMP"'
        'xcrun simctl launch --terminate-running-process "$DEV" "$BUNDLE" -autoscreenshot "$REMOTEBMP" "$SETTLEMS" -autoquit'
        'i=0; while [ $i -lt $TIMEOUTSEC ]; do if [ -f "$REMOTEBMP" ]; then break; fi; sleep 1; i=$((i+1)); done'
        'xcrun simctl terminate "$DEV" "$BUNDLE" 2>/dev/null || true'
        'if [ ! -f "$REMOTEBMP" ]; then echo "app never wrote $REMOTEBMP"; exit 1; fi'
    )
    $scriptPath = Join-Path $outputDir 'ios_capture.sh'
    [IO.File]::WriteAllText($scriptPath, ($lines -join "`n") + "`n")
    & scp -q $scriptPath "$MacHost`:/tmp/proton_harness_capture.sh"
    if ($LASTEXITCODE -ne 0) { throw 'scp of capture script failed' }
    & ssh $MacHost "bash /tmp/proton_harness_capture.sh $appName $remoteBmp $settleMs $timeoutSec '$IosSimDevice'"
    if ($LASTEXITCODE -ne 0) { throw 'remote iOS capture failed (see output above)' }
    & scp -q "$MacHost`:$remoteBmp" $bmpOutPath
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $bmpOutPath)) { throw "scp of $remoteBmp failed" }
    & scp -q "$MacHost`:$remoteBmp.perf.txt" "$bmpOutPath.perf.txt" 2>$null # best effort
}

# ios -IosDevice: runs on a real paired device via devicectl. The app writes
# to its sandbox Documents (unique name per run, since old files persist
# there) and devicectl copies it out.

function Invoke-IosDeviceCapture([string]$appName, [int]$settleMs, [string]$bmpOutPath)
{
    $timeoutSec = [int](($settleMs + 60000) / 1000)
    $lines = @(
        'set -e'
        'APPNAME=$1; SETTLEMS=$2; TIMEOUTSEC=$3; DEVID=$4'
        'if [ -z "$DEVID" ]; then DEVID=$(xcrun devicectl list devices | grep -m1 "available (paired)" | grep -oE "[A-F0-9-]{36}"); fi'
        'if [ -z "$DEVID" ]; then echo "no paired iOS device visible to devicectl"; exit 1; fi'
        'APP=$(find ~/proton_warncheck/$APPNAME -path "*Debug-iphoneos*" -name "$APPNAME.app" | head -1)'
        'if [ -z "$APP" ]; then echo "no device .app built - run the harness with -PrepareMac -IosDevice"; exit 1; fi'
        'BUNDLE=$(/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$APP/Info.plist")'
        'SHOT=shot_$(date +%s).bmp' # unique per run: old files persist in the sandbox
        'xcrun devicectl device install app --device $DEVID "$APP" >/dev/null'
        'xcrun devicectl device process launch --terminate-existing --device $DEVID "$BUNDLE" -- -autoscreenshot "$SHOT" "$SETTLEMS" -autoquit >/dev/null'
        'rm -f /tmp/proton_device_shot.bmp /tmp/proton_device_shot.bmp.perf.txt'
        'i=0'
        'while [ $i -lt $TIMEOUTSEC ]; do'
        '  sleep 2'
        '  if xcrun devicectl device copy from --device $DEVID --domain-type appDataContainer --domain-identifier "$BUNDLE" --source "Documents/$SHOT" --destination /tmp/proton_device_shot.bmp >/dev/null 2>&1; then break; fi'
        '  i=$((i+2))'
        'done'
        'if [ ! -f /tmp/proton_device_shot.bmp ]; then echo "never received $SHOT from the device (is it unlocked?)"; exit 1; fi'
        'xcrun devicectl device copy from --device $DEVID --domain-type appDataContainer --domain-identifier "$BUNDLE" --source "Documents/$SHOT.perf.txt" --destination /tmp/proton_device_shot.bmp.perf.txt >/dev/null 2>&1 || true'
    )
    $scriptPath = Join-Path $outputDir 'ios_device_capture.sh'
    [IO.File]::WriteAllText($scriptPath, ($lines -join "`n") + "`n")
    & scp -q $scriptPath "$MacHost`:/tmp/proton_harness_devcapture.sh"
    if ($LASTEXITCODE -ne 0) { throw 'scp of device capture script failed' }
    & ssh $MacHost "bash /tmp/proton_harness_devcapture.sh $appName $settleMs $timeoutSec '$IosDeviceId'"
    if ($LASTEXITCODE -ne 0) { throw 'remote iOS device capture failed (see output above)' }
    & scp -q "$MacHost`:/tmp/proton_device_shot.bmp" $bmpOutPath
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $bmpOutPath)) { throw 'scp of device screenshot failed' }
    & scp -q "$MacHost`:/tmp/proton_device_shot.bmp.perf.txt" "$bmpOutPath.perf.txt" 2>$null # best effort
}

# ---------------------------------------------------------------------------
# android target: runs on the first device/emulator adb reports. The app must
# already be installed (gradle installDebug); we pass the parms as an intent
# extra (see SharedActivity.onCreate), the app writes the BMP to its internal
# files dir, and we pull it out with run-as (works for debuggable builds).

function Invoke-AndroidCapture([string]$package, [string]$activity, [int]$settleMs, [string]$bmpOutPath)
{
    $remoteBmp = "/data/data/$package/files/proton_harness.bmp"
    & adb shell am force-stop $package 2>$null | Out-Null
    & adb shell run-as $package rm -f files/proton_harness.bmp 2>$null | Out-Null
    & adb shell am start -n "$package/$activity" --es parms "'-autoscreenshot $remoteBmp $settleMs -autoquit'" | Out-Null
    $deadline = (Get-Date).AddMilliseconds($settleMs + 60000)
    $found = $false
    while ((Get-Date) -lt $deadline)
    {
        Start-Sleep -Seconds 1
        $probe = & adb shell run-as $package ls files/proton_harness.bmp 2>$null
        if ($probe -match 'proton_harness.bmp') { $found = $true; break }
    }
    & adb shell am force-stop $package 2>$null | Out-Null
    if (-not $found) { throw "app never wrote $remoteBmp (check adb logcat)" }
    & adb exec-out run-as $package cat files/proton_harness.bmp > $bmpOutPath
    if (-not (Test-Path $bmpOutPath) -or (Get-Item $bmpOutPath).Length -lt 1000) { throw "adb pull of screenshot failed" }
    & adb exec-out run-as $package cat files/proton_harness.bmp.perf.txt > "$bmpOutPath.perf.txt" 2>$null # best effort
}

$scenarios = Import-PowerShellDataFile (Join-Path $PSScriptRoot 'scenarios.psd1')
$goldenDir = Join-Path $PSScriptRoot 'goldens'
$outputDir = Join-Path $PSScriptRoot 'output'
New-Item -ItemType Directory -Force $goldenDir | Out-Null
New-Item -ItemType Directory -Force $outputDir | Out-Null

$results = @()
$anyFailed = $false

# Kill leftovers from earlier runs (a crashed launch can sit forever behind an
# "Application Error" dialog).
Get-Process -ErrorAction SilentlyContinue | Where-Object {
    $_.Path -and $_.Path -like "$RepoRoot\*\bin\*"
} | ForEach-Object {
    Write-Host "KILL  leftover $($_.Name) (pid $($_.Id))" -ForegroundColor Yellow
    Stop-Process -Id $_.Id -Force -Confirm:$false
}

if ($Target -eq 'android')
{
    $devices = (& adb devices) -match "`tdevice$"
    if (-not $devices) { throw 'android target: no device/emulator visible to adb (plug one in or start an emulator, and gradle installDebug the app first)' }
}

if ($Target -eq 'ios' -and $PrepareMac)
{
    Write-Host "Syncing tracked tree to $MacHost and building simulator apps..." -ForegroundColor Cyan
    Push-Location $RepoRoot
    try
    {
        cmd /c "git ls-files -z | tar czf `"%TEMP%\proton_harness_mac.tgz`" --null -T -"
        if ($LASTEXITCODE -ne 0) { throw 'tar of tracked files failed' }
        & scp -q "$env:TEMP\proton_harness_mac.tgz" "$MacHost`:/tmp/"
        & ssh $MacHost 'rm -rf ~/proton_warncheck && mkdir -p ~/proton_warncheck && tar xzf /tmp/proton_harness_mac.tgz -C ~/proton_warncheck'
        if ($LASTEXITCODE -ne 0) { throw 'remote extract failed' }
        foreach ($entry in $scenarios.Apps)
        {
            if ($entry.Name -notlike $App -or -not $entry.ContainsKey('IosProject')) { continue }
            $proj = $entry.IosProject -replace '\\', '/'
            if ($IosDevice)
            {
                Write-Host "  xcodebuild $($entry.Name) (iphoneos, signed)..."
                $unlock = if ($MacKeychainPassword) { "security unlock-keychain -p '$MacKeychainPassword' ~/Library/Keychains/login.keychain-db && " } else { '' }
                & ssh $MacHost "$unlock cd ~/proton_warncheck && xcodebuild -project $proj -target $($entry.Name) -configuration Debug -sdk iphoneos CODE_SIGN_STYLE=Automatic DEVELOPMENT_TEAM=7DA5SJEYK8 -allowProvisioningUpdates build 2>&1 | grep -E 'error|BUILD' | tail -3"
                if ($LASTEXITCODE -ne 0) { throw "iOS device build of $($entry.Name) failed (keychain locked? pass -MacKeychainPassword, see agents_secret.md)" }
            }
            else
            {
                Write-Host "  xcodebuild $($entry.Name) (iphonesimulator)..."
                & ssh $MacHost "cd ~/proton_warncheck && xcodebuild -project $proj -target $($entry.Name) -configuration Debug -sdk iphonesimulator CODE_SIGNING_ALLOWED=NO build 2>&1 | grep -E 'error|BUILD' | tail -3"
                if ($LASTEXITCODE -ne 0) { throw "iOS build of $($entry.Name) failed" }
            }
        }
    }
    finally { Pop-Location }
}

foreach ($entry in $scenarios.Apps)
{
    if ($entry.Name -notlike $App) { continue }

    if ($Target -eq 'html5')
    {
        if (-not $entry.ContainsKey('Html5Page'))
        {
            Write-Host "SKIP  $($entry.Name): no Html5Page defined in scenarios.psd1" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'no Html5Page' }
            continue
        }
        $page = Join-Path $RepoRoot $entry.Html5Page
        if (-not (Test-Path $page))
        {
            Write-Host "SKIP  $($entry.Name): $page not found (run the app's html5 build_release.bat first)" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'html5 page not built' }
            continue
        }
    }
    elseif ($Target -eq 'ios')
    {
        if (-not $entry.ContainsKey('IosProject'))
        {
            Write-Host "SKIP  $($entry.Name): no IosProject defined in scenarios.psd1" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'no IosProject' }
            continue
        }
    }
    elseif ($Target -eq 'android')
    {
        if (-not $entry.ContainsKey('AndroidPackage'))
        {
            Write-Host "SKIP  $($entry.Name): no AndroidPackage defined in scenarios.psd1" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'no AndroidPackage' }
            continue
        }
    }
    else
    {
        if ($ShaderPipe -and -not ($entry.ContainsKey('ShaderReady') -and $entry.ShaderReady))
        {
            Write-Host "SKIP  $($entry.Name): not marked ShaderReady in scenarios.psd1 (its app-level raw GL needs the compatibility shim first)" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'not ShaderReady' }
            continue
        }
        if (-not $ShaderPipe -and $entry.ContainsKey('RequiresShaderPipe') -and $entry.RequiresShaderPipe)
        {
            Write-Host "SKIP  $($entry.Name): shader-pipeline-only scenario (run with -ShaderPipe)" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'needs -ShaderPipe' }
            continue
        }
        $exe = Join-Path $RepoRoot $entry.Exe
        if (-not (Test-Path $exe))
        {
            Write-Host "SKIP  $($entry.Name): $exe not found (separate repo not checked out, or not built)" -ForegroundColor Yellow
            $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'exe missing' }
            continue
        }
    }

    # v1 supports exactly one capture step per app, driven by -autoscreenshot
    $captureSteps = @($entry.Steps | Where-Object { $_.Action -eq 'capture' })
    if ($captureSteps.Count -ne 1) { throw "$($entry.Name): expected exactly one capture step" }
    $step = $captureSteps[0]

    Write-Host "RUN   $($entry.Name)" -ForegroundColor Cyan
    try
    {
        $prefix = if ($Target -eq 'win') { '' } elseif ($Target -eq 'ios' -and $IosDevice) { 'iosdev_' } else { "$($Target)_" }
        $shotName = "$prefix$($entry.Name)_$($step.Name).png"
        $bmpPath = Join-Path $outputDir "$prefix$($entry.Name)_$($step.Name).bmp"
        if ($bmpPath -match ' ') { throw "path contains spaces; Proton's Windows parm tokenizer splits on them: $bmpPath" }
        Remove-Item $bmpPath -Force -ErrorAction SilentlyContinue

        if ($Target -eq 'html5')
        {
            $pageFull = Join-Path $RepoRoot $entry.Html5Page
            $html5Extra = if ($entry.ContainsKey('ExtraParms')) { $entry.ExtraParms } else { '' }
            Invoke-Html5Capture (Split-Path $pageFull) (Split-Path $pageFull -Leaf) $entry.SettleMs $bmpPath $html5Extra
        }
        elseif ($Target -eq 'ios')
        {
            if ($IosDevice) { Invoke-IosDeviceCapture $entry.Name $entry.SettleMs $bmpPath }
            else { Invoke-IosCapture $entry.IosProject $entry.Name $entry.SettleMs $bmpPath }
        }
        elseif ($Target -eq 'android')
        {
            $activity = if ($entry.ContainsKey('AndroidActivity')) { $entry.AndroidActivity } else { '.Main' }
            Invoke-AndroidCapture $entry.AndroidPackage $activity $entry.SettleMs $bmpPath
        }
        else
        {
            $appArgs = @('-autoscreenshot', $bmpPath, [string]$entry.SettleMs, '-autoquit')
            if ($ShaderPipe) { $appArgs = @('-shaderpipeline') + $appArgs }
            if ($entry.ContainsKey('ExtraParms')) { $appArgs = ($entry.ExtraParms -split ' ') + $appArgs }
            $proc = Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe) -PassThru -ArgumentList $appArgs

            $deadline = (Get-Date).AddMilliseconds($entry.SettleMs + 60000)
            while ((Get-Date) -lt $deadline -and -not $proc.HasExited) { Start-Sleep -Milliseconds 250; $proc.Refresh() }
            if (-not $proc.HasExited)
            {
                Stop-Process -Id $proc.Id -Force -Confirm:$false
                Write-Host "  WARN  $($entry.Name) didn't quit on its own, killed" -ForegroundColor Yellow
            }
            if (-not (Test-Path $bmpPath)) { throw "app never wrote $bmpPath (check $((Split-Path $exe))\log.txt)" }
        }

        $ignore = if ($step.ContainsKey('IgnoreRects')) { $step.IgnoreRects } elseif ($entry.ContainsKey('IgnoreRects')) { $entry.IgnoreRects } else { $null }
        $capturePath = Join-Path $outputDir $shotName
        Convert-ShotToPng $bmpPath $capturePath $ignore
        Remove-Item $bmpPath -Force

        if ($Mode -eq 'golden')
        {
            Copy-Item $capturePath (Join-Path $goldenDir $shotName) -Force
            Write-Host "  GOLDEN  $shotName"
            $results += @{ App = $entry.Name; Step = $step.Name; Status = 'GOLDEN'; Note = '' }
        }
        else
        {
            $goldenPath = Join-Path $goldenDir $shotName
            if (-not (Test-Path $goldenPath))
            {
                Write-Host "  NOGOLD  $shotName" -ForegroundColor Yellow
                $results += @{ App = $entry.Name; Step = $step.Name; Status = 'NOGOLD'; Note = 'run -Mode golden first' }
                continue
            }
            $tol = if ($step.ContainsKey('ChannelTol')) { $step.ChannelTol } elseif ($entry.ContainsKey('ChannelTol')) { $entry.ChannelTol } else { 12 }
            $maxPct = if ($step.ContainsKey('MaxDiffPct')) { $step.MaxDiffPct } elseif ($entry.ContainsKey('MaxDiffPct')) { $entry.MaxDiffPct } else { 0.5 }
            if ($ShaderPipe) { $maxPct = if ($entry.ContainsKey('ShaderMaxDiffPct')) { $entry.ShaderMaxDiffPct } else { 3.0 } } # different rasterization path; small edge drift expected
            $cmp = Compare-Shots $goldenPath $capturePath $tol (Join-Path $outputDir "$($entry.Name)_$($step.Name)_DIFF.png")
            $pass = $cmp.DiffPct -le $maxPct
            if (-not $pass) { $anyFailed = $true }
            $status = if ($pass) { 'PASS' } else { 'FAIL' }
            $color = if ($pass) { 'Green' } else { 'Red' }
            Write-Host ("  {0}    {1}  ({2:N3}% differ, limit {3}%; {4})" -f $status, $shotName, $cmp.DiffPct, $maxPct, $cmp.Note) -ForegroundColor $color
            $results += @{ App = $entry.Name; Step = $step.Name; Status = $status; Note = ("{0:N3}%" -f $cmp.DiffPct) }
        }

        # Speed check: the engine writes a perf sidecar (frames rendered on the
        # locked timestep vs the wall clock they took). The first sighting (or a
        # golden run) records a baseline next to the goldens; after that,
        # dropping below MinFpsRatio of the baseline (default 0.5) fails - a
        # wide margin, meant to catch "everything got horribly slower" bugs
        # without flaking on system load. Note the fps ceiling per target:
        # vsync is uncapped on win, but html5/ios are capped near 60 by the
        # browser/display, so those only catch drops below the cap.
        $perfPath = "$bmpPath.perf.txt"
        if ($ShaderPipe -and (Test-Path $perfPath))
        {
            # informational only: shader-path perf isn't compared against the legacy baselines
            $perfRaw = Get-Content $perfPath -Raw
            if ($perfRaw -match 'fps=([0-9.]+)') { $fps = $Matches[1] } else { $fps = '?' }
            if ($perfRaw -match 'engineMS=([0-9.]+)') { $engMS = $Matches[1] } else { $engMS = '?' }
            Write-Host "  PERF    info  $fps fps, $engMS engine ms/frame (shader path, not gated)"
        }
        elseif (Test-Path $perfPath)
        {
            $perfRaw = Get-Content $perfPath -Raw
            $fps = 0.0; $engMS = 0.0
            if ($perfRaw -match 'fps=([0-9.]+)') { $fps = [double]$Matches[1] }
            if ($perfRaw -match 'engineMS=([0-9.]+)') { $engMS = [double]$Matches[1] }
            $perfBasePath = Join-Path $goldenDir ($shotName -replace '\.png$', '.perf.txt')
            if ($fps -le 0) { }
            elseif ($Mode -eq 'golden' -or -not (Test-Path $perfBasePath))
            {
                Copy-Item $perfPath $perfBasePath -Force
                Write-Host ('  PERF    baseline recorded: {0:0.0} fps, {1:0.000} engine ms/frame' -f $fps, $engMS)
            }
            else
            {
                $baseRaw = Get-Content $perfBasePath -Raw
                $baseFps = 0.0; $baseEngMS = 0.0
                if ($baseRaw -match 'fps=([0-9.]+)') { $baseFps = [double]$Matches[1] }
                if ($baseRaw -match 'engineMS=([0-9.]+)') { $baseEngMS = [double]$Matches[1] }
                $minRatio = if ($entry.ContainsKey('MinFpsRatio')) { $entry.MinFpsRatio } else { 0.5 }
                # two signals: wall fps floor (catches anything, but display/browser caps
                # limit its ceiling), and engine ms/frame (vsync-immune; only meaningful
                # above 1ms so sub-ms cache noise can't flake)
                $fpsFail = ($baseFps -gt 0 -and $fps -lt $baseFps * $minRatio)
                $engFail = ($baseEngMS -gt 0 -and $engMS -gt 1.0 -and $engMS -gt $baseEngMS * 2.0 -and $engMS -gt $baseEngMS + 0.5)
                if ($fpsFail -or $engFail)
                {
                    $anyFailed = $true
                    Write-Host ('  PERF    FAIL  {0:0.0} fps / {1:0.000} eng-ms vs baseline {2:0.0} / {3:0.000}' -f $fps, $engMS, $baseFps, $baseEngMS) -ForegroundColor Red
                    $results += @{ App = $entry.Name; Step = "$($step.Name)/perf"; Status = 'FAIL'; Note = ('{0:0.0}fps {1:0.000}ms vs {2:0.0}fps {3:0.000}ms' -f $fps, $engMS, $baseFps, $baseEngMS) }
                }
                else
                {
                    Write-Host ('  PERF    ok    {0:0.0} fps, {1:0.000} engine ms/frame (baseline {2:0.0} / {3:0.000})' -f $fps, $engMS, $baseFps, $baseEngMS)
                }
            }
        }
    }
    catch
    {
        Write-Host "ERROR $($entry.Name): $($_.Exception.Message)" -ForegroundColor Red
        $results += @{ App = $entry.Name; Step = '-'; Status = 'ERROR'; Note = $_.Exception.Message }
        $anyFailed = $true
    }
}

Write-Host "`n===== Summary ($Mode mode) ====="
foreach ($r in $results) { Write-Host ("{0,-8} {1,-18} {2,-10} {3}" -f $r.Status, $r.App, $r.Step, $r.Note) }
if ($Mode -eq 'test' -and $anyFailed) { exit 1 }
exit 0
