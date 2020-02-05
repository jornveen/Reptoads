@echo off
cls
echo Converting textures...
for /R %cd%\..\data\Client\data\Textures\ %%G IN (*.png, *.bmp, *.jpg, *.dds, *.hdr) DO (
echo [CHECK] \..\data\Client\data\Gnf\%%~nG
If not exist "%cd%\..\data\Client\data\Gnf\%%~nG.gnf" (
orbis-image2gnf.exe -q -i %%G -f BC1Unorm -o %cd%\..\data\Client\data\Gnf\%%~nG.gnf
echo [STATUS] Texture successfully converted.
) ELSE (
echo [STATUS] Texture is up to date.
)
)
echo [STATUS] DONE
