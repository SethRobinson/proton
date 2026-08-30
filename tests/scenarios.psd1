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
# Exe paths are relative to the repo root. Per Seth: only apps tracked in the
# proton repo plus RTDink, RTDScroll, and RTMindWall may be used for testing.
# RTDink and RTMindWall live in sibling checkouts/folders; the harness skips
# any app whose exe is missing. Build configs used: Debug GL (Win32 for
# RTBareBones; x64 for the rest). Several Win32 configs are stale: they hit
# 0xc000007b at launch loading 64-bit DLLs from bin, or have outdated file
# exclusion lists, so prefer x64.

Apps = @(
    @{
        Name = 'RTBareBones'
        Exe = 'RTBareBones\bin\RTBareBones_Debug GL_Win32.exe'
        Html5Page = 'RTBareBones\html5\RTBareBones.html'
        IosProject = 'RTBareBones\RTBareBones.xcodeproj'
        AndroidPackage = 'com.rtsoft.RTAndroidApp'
        ShaderReady = $true
        SettleMs = 6000
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
    @{
        # The render-to-texture + custom shader demo (shader pipeline only, see
        # RenderRTTDemoIfRequested in RTBareBones App.cpp). Its golden captures
        # the RTT surface blitted plain and through the wavy shader.
        Name = 'RTBareBonesRTT'
        Exe = 'RTBareBones\bin\RTBareBones_Debug GL_Win32.exe'
        Html5Page = 'RTBareBones\html5\RTBareBones.html'
        ShaderReady = $true
        RequiresShaderPipe = $true
        ExtraParms = '-rttdemo'
        SettleMs = 6000
        Steps = @(
            @{ Action = 'capture'; Name = 'rttdemo' }
        )
    }
    @{
        # The custom-shader example app: renders a scene to a texture and
        # blits it through a GLSL post-process effect (default: wavy).
        Name = 'RTShader'
        Exe = 'RTShader\bin\RTShader_Debug GL_Win32.exe'
        Html5Page = 'RTShader\html5\RTShader.html'
        IosProject = 'RTShader\RTShader.xcodeproj'
        ShaderReady = $true
        RequiresShaderPipe = $true
        SettleMs = 6000
        Steps = @(
            @{ Action = 'capture'; Name = 'wavy' }
        )
    }
    @{
        # RTShader's 3D mode: a textured spinning cube drawn with classic
        # fixed-function-style GL (perspective + depth + client arrays) via
        # the compatibility shim - the only suite coverage of that 3D path
        # beyond RTBareBones' flat triangle.
        Name = 'RTShaderCube'
        Exe = 'RTShader\bin\RTShader_Debug GL_Win32.exe'
        Html5Page = 'RTShader\html5\RTShader.html'
        ShaderReady = $true
        RequiresShaderPipe = $true
        ExtraParms = '-effect 5'
        SettleMs = 6000
        Steps = @(
            @{ Action = 'capture'; Name = 'cube' }
        )
    }
    @{
        Name = 'RTSimpleApp'
        Exe = 'RTSimpleApp\bin\RTSimpleApp_Debug_GL.exe'
        Html5Page = 'RTSimpleApp\html5\RTSimpleApp.html'
        ShaderReady = $true
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTLooneyLadders'
        Exe = 'RTLooneyLadders\bin\RTLooneyLadders_Debug GL_x64.exe'
        ShaderReady = $true
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTDink'
        Exe = 'RTDink\bin\winRTDink_Debug GL.exe'
        ShaderReady = $true
        # Dink's menu only appears after its online update check answers, a
        # WALL-clock event; uncapped captures blast to the capture tick in
        # under a second so the menu isn't up yet.  -autoscreenshotfps makes
        # the engine honor fps limits during the capture, restoring a sane
        # wall pace (Dink itself then sets 60).
        ExtraParms = '-autoscreenshotfps 120'
        SettleMs = 12000
        Steps = @(
            @{ Action = 'capture'; Name = 'mainmenu' }
        )
    }
    @{
        Name = 'RTMindWall'
        Exe = 'RTMindWall\bin\winRTMindWall_Debug GL.exe'
        ShaderReady = $true
        SettleMs = 8000
        Steps = @(
            @{ Action = 'capture'; Name = 'main' }
        )
    }
)
}

