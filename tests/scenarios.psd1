@{
# Renderer regression scenarios. Each app is launched from its bin folder with
# Proton's -autoscreenshot parm; the app writes its framebuffer to a BMP once
# its own timer passes SettleMs, then quits. Because capture time is app-clock
# based, tick-driven animations land in (nearly) the same pose every run; only
# wall-clock content (the FPS counter) and randomized content (particles) need
# IgnoreRects masks. ChannelTol = per-channel difference that still counts as
# "same pixel"; MaxDiffPct = % of pixels allowed to differ.
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
        # Mask: FPS/timer text. The capture can land one frame apart between
        # runs, so the spinning triangle/logo edges drift ~1%; 3% still catches
        # any real rendering change.
        MaxDiffPct = 3.0
        IgnoreRects = @( @(0, 0, 1024, 48) )
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
    @{
        Name = 'RTSimpleApp'
        Exe = 'RTSimpleApp\bin\RTSimpleApp_Debug_GL.exe'
        SettleMs = 8000
        # Mask: FPS/timer text
        IgnoreRects = @( @(0, 0, 640, 48) )
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTLooneyLadders'
        Exe = 'RTLooneyLadders\bin\RTLooneyLadders_Debug GL_x64.exe'
        SettleMs = 8000
        # Mask: FPS/timer text
        IgnoreRects = @( @(0, 0, 420, 26) )
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTDink'
        Exe = 'RTDink\bin\winRTDink_Debug GL.exe'
        SettleMs = 12000
        # Mask: FPS/timer text. The title-screen fire animation proved fully
        # tick-deterministic (0.000% run-to-run), so it needs no mask. Note:
        # expects a previous save so the "Continue your last session?" prompt
        # appears; goldens are per-machine anyway.
        IgnoreRects = @( @(0, 0, 520, 42) )
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'BlipArcade'
        Exe = 'BlipArcade\bin\BlipArcade_debug.exe'
        SettleMs = 8000
        # The little animated "blip" around the logo may not be perfectly
        # reproducible; allow a sliver of drift.
        MaxDiffPct = 1.5
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
    @{
        Name = 'RTMindWall'
        Exe = 'RTMindWall\bin\winRTMindWall_Debug GL.exe'
        SettleMs = 8000
        # The 3D cube wall advances ~one frame between runs (thin edge drift,
        # ~1.5%); 4% still catches any real rendering change.
        MaxDiffPct = 4.0
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
)
}
