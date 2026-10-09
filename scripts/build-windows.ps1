param([switch]$Run)
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
foreach ($tool in @('git', 'cmake')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) { throw "Instale $tool e adicione ao PATH." }
}
cmake --preset windows
if ($LASTEXITCODE -ne 0) { throw 'CMake configure falhou.' }
cmake --build --preset windows --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build falhou.' }
ctest --preset windows
if ($LASTEXITCODE -ne 0) { throw 'Testes falharam.' }
cmake --install build/windows --config Release --prefix build/package-windows --component Runtime
if ($LASTEXITCODE -ne 0) { throw 'Empacotamento falhou.' }
Compress-Archive -Path build/package-windows/* -DestinationPath build/MeuGalinheiro-Windows.zip -Force
Write-Host 'Jogo: build/package-windows/meu_galinheiro.exe'
Write-Host 'Pacote: build/MeuGalinheiro-Windows.zip'
if ($Run) { & './build/package-windows/meu_galinheiro.exe' }
