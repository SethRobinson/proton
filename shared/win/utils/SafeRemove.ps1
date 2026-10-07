# Delete only an explicitly named child of an absolute, validated root.
# Batch callers must check ERRORLEVEL and pass a literal RelativePath where possible.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][AllowEmptyString()][string]$Root,
    [Parameter(Mandatory=$true)][AllowEmptyString()][string]$RelativePath,
    [switch]$Recurse,
    [switch]$FilesOnly
)
$ErrorActionPreference = 'Stop'

try {
    if ([string]::IsNullOrWhiteSpace($Root) -or $Root -notmatch '^[A-Za-z]:[\\/]') {
        throw 'Cleanup root must be a nonempty absolute drive path.'
    }
    if ($Root.IndexOfAny([char[]]'*?[]') -ge 0) { throw 'Wildcard cleanup root refused.' }
    $rootPath = [IO.Path]::GetFullPath($Root).TrimEnd('\', '/')
    if ($rootPath.Length -le 3) { throw 'Cleanup of a drive root refused.' }
    if ([string]::IsNullOrWhiteSpace($RelativePath) -or [IO.Path]::IsPathRooted($RelativePath)) {
        throw 'Cleanup requires a nonempty relative child path.'
    }
    $parts = $RelativePath -split '[\\/]'
    foreach ($part in $parts) {
        if ([string]::IsNullOrWhiteSpace($part) -or $part -in @('.', '..') -or
            $part -match '[:\[\]]' -or $part -match '[. ]$') { throw 'Unsafe cleanup child path.' }
    }
    $leaf = $parts[-1]
    $hasPattern = $leaf.IndexOfAny([char[]]'*?') -ge 0
    if ($hasPattern -and -not $FilesOnly) { throw 'Wildcards require FilesOnly.' }
    if ($parts.Count -gt 1 -and (($parts[0..($parts.Count-2)] -join '\') -match '[*?]')) {
        throw 'Wildcards in cleanup parent directories refused.'
    }
    # .NET Framework's GetFullPath rejects wildcard characters. Normalize the
    # literal parent separately, then append the checked leaf pattern.
    $relativeParent = if ($parts.Count -gt 1) { $parts[0..($parts.Count-2)] -join '\' } else { '' }
    $literalParent = [IO.Path]::GetFullPath([IO.Path]::Combine($rootPath, $relativeParent))
    $target = [IO.Path]::Combine($literalParent, $leaf)
    if (-not $target.StartsWith($rootPath + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Cleanup target escaped its root.'
    }
    # Check each existing ancestor before traversing it. Never follow a junction.
    $parent = if ($hasPattern) { [IO.Path]::GetDirectoryName($target) } else { $target }
    $probe = [IO.Path]::GetPathRoot($parent)
    foreach ($part in $parent.Substring($probe.Length).Split('\')) {
        $probe = [IO.Path]::Combine($probe, $part)
        if (Test-Path -LiteralPath $probe) {
            if ((Get-Item -LiteralPath $probe -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) {
                throw "Cleanup through a link/junction refused: $probe"
            }
        }
    }
    if (-not (Test-Path -LiteralPath $parent)) { exit 0 }
    # Preflight the entire tree before making any changes.
    $entries = @()
    if ($Recurse) {
        $pending = New-Object 'System.Collections.Generic.Stack[string]'
        $pending.Push($parent)
        while ($pending.Count) {
            foreach ($item in Get-ChildItem -LiteralPath $pending.Pop() -Force) {
                if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Cleanup link refused: $($item.FullName)" }
                $entries += $item
                if ($item.PSIsContainer) { $pending.Push($item.FullName) }
            }
        }
    }
    if ($hasPattern) {
        if (-not $Recurse) { $entries = @(Get-ChildItem -LiteralPath $parent -Force) }
        foreach ($item in $entries) {
            if (-not $item.PSIsContainer -and $item.Name -like $leaf) {
                if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Cleanup link refused.' }
                Remove-Item -LiteralPath $item.FullName -Force
            }
        }
    } else {
        $item = Get-Item -LiteralPath $target -Force
        if ($FilesOnly -and $item.PSIsContainer) { throw 'Expected a file, found a directory.' }
        Remove-Item -LiteralPath $target -Recurse:$Recurse -Force
    }
} catch {
    Write-Error $_
    exit 1
}
