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
    [string]$App = '*',
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

foreach ($entry in $scenarios.Apps)
{
    if ($entry.Name -notlike $App) { continue }
    $exe = Join-Path $RepoRoot $entry.Exe
    if (-not (Test-Path $exe))
    {
        Write-Host "SKIP  $($entry.Name): $exe not found (separate repo not checked out, or not built)" -ForegroundColor Yellow
        $results += @{ App = $entry.Name; Step = '-'; Status = 'SKIP'; Note = 'exe missing' }
        continue
    }

    # v1 supports exactly one capture step per app, driven by -autoscreenshot
    $captureSteps = @($entry.Steps | Where-Object { $_.Action -eq 'capture' })
    if ($captureSteps.Count -ne 1) { throw "$($entry.Name): expected exactly one capture step" }
    $step = $captureSteps[0]

    Write-Host "RUN   $($entry.Name)" -ForegroundColor Cyan
    try
    {
        $shotName = "$($entry.Name)_$($step.Name).png"
        $bmpPath = Join-Path $outputDir "$($entry.Name)_$($step.Name).bmp"
        if ($bmpPath -match ' ') { throw "path contains spaces; Proton's Windows parm tokenizer splits on them: $bmpPath" }
        Remove-Item $bmpPath -Force -ErrorAction SilentlyContinue

        $proc = Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe) -PassThru `
            -ArgumentList @('-autoscreenshot', $bmpPath, [string]$entry.SettleMs, '-autoquit')

        $deadline = (Get-Date).AddMilliseconds($entry.SettleMs + 60000)
        while ((Get-Date) -lt $deadline -and -not $proc.HasExited) { Start-Sleep -Milliseconds 250; $proc.Refresh() }
        if (-not $proc.HasExited)
        {
            Stop-Process -Id $proc.Id -Force -Confirm:$false
            Write-Host "  WARN  $($entry.Name) didn't quit on its own, killed" -ForegroundColor Yellow
        }
        if (-not (Test-Path $bmpPath)) { throw "app never wrote $bmpPath (check $((Split-Path $exe))\log.txt)" }

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
            $cmp = Compare-Shots $goldenPath $capturePath $tol (Join-Path $outputDir "$($entry.Name)_$($step.Name)_DIFF.png")
            $pass = $cmp.DiffPct -le $maxPct
            if (-not $pass) { $anyFailed = $true }
            $status = if ($pass) { 'PASS' } else { 'FAIL' }
            $color = if ($pass) { 'Green' } else { 'Red' }
            Write-Host ("  {0}    {1}  ({2:N3}% differ, limit {3}%; {4})" -f $status, $shotName, $cmp.DiffPct, $maxPct, $cmp.Note) -ForegroundColor $color
            $results += @{ App = $entry.Name; Step = $step.Name; Status = $status; Note = ("{0:N3}%" -f $cmp.DiffPct) }
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
