@{
# Renderer regression scenarios. Each app is launched from its bin folder with
# Proton's -autoscreenshot parm; the app writes its framebuffer to a BMP once
# its own timer passes SettleMs, then quits. The parm also puts the engine in
# deterministic mode (locked 16ms timestep, timeline zeroed at Init, fixed
# rand seed), so animations and particles are in identical poses every run and
# captures are pixel-exact: even the FPS counter reads a constant 62. No app
# currently needs an IgnoreRects mask, but per-app/per-step IgnoreRects
# (@(x,y,w,h) rects painted magenta before compare) remain supported for any
# future wall-clock-dependent content. ChannelTol = per-channel difference that
# still counts as "same pixel"; MaxDiffPct = % of pixels allowed to differ
# (default 0.5). RTDink expects a previous save to exist (it captures the
# "Continue your last session?" prompt).
#
# Exe paths are relative to the repo root. RTDink, BlipArcade, and RTMindWall
# live in sibling checkouts/folders (separate repos); the harness skips any
# app whose exe is missing. Build configs used: Debug GL (Win32 for
# RTBareBones; x64 for the rest). Several Win32 configs are stale: they hit
# 0xc000007b at launch loading 64-bit DLLs from bin, or have outdated file
# exclusion lists, so prefer x64.

Apps = @(
    @{
        Name = 'RTBareBones'
        Exe = 'RTBareBones\bin\RTBareBones_Debug GL_Win32.exe'
        SettleMs = 6000
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
    @{
        Name = 'RTSimpleApp'
        Exe = 'RTSimpleApp\bin\RTSimpleApp_Debug_GL.exe'
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTLooneyLadders'
        Exe = 'RTLooneyLadders\bin\RTLooneyLadders_Debug GL_x64.exe'
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTDink'
        Exe = 'RTDink\bin\winRTDink_Debug GL.exe'
        SettleMs = 12000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'BlipArcade'
        Exe = 'BlipArcade\bin\BlipArcade_debug.exe'
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
    @{
        Name = 'RTMindWall'
        Exe = 'RTMindWall\bin\winRTMindWall_Debug GL.exe'
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
)
}

