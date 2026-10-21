# Windows PowerShell 5.1 / PowerShell 7. Requires Python 3.10+ and UE5.6 Windows.
# Preserve V80 maps; add/validate V83 Rurik and retain V81/V82 startup checks.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$EngineRoot,
    [string]$Project = (Join-Path $PSScriptRoot '..\AskirRPG.uproject'),
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$Python = 'python',
    [switch]$CheckOnly,
    [switch]$UseExistingMap
)
$ErrorActionPreference = 'Stop'
$runner = Join-Path $PSScriptRoot '..\Tools\run_unreal_v83.py'
if (-not (Test-Path -LiteralPath $runner -PathType Leaf)) { throw "Runner missing: $runner" }
$arguments = @($runner, '--engine-root', $EngineRoot, '--project', $Project, '--output', $OutputDirectory)
if ($CheckOnly) { $arguments += '--check-only' }
if ($UseExistingMap) { $arguments += '--use-existing-map' }
& $Python @arguments
$result = $LASTEXITCODE
if ($result -ne 0) {
    Write-Error -ErrorAction Continue "ASKIR V83 pipeline stopped (exit $result). See $(Join-Path $OutputDirectory 'pipeline.json')."
}
exit $result
