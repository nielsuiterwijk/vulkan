IF EXIST "%cd%/intermediate/" RMDIR /S /Q "%cd%/intermediate/"
IF NOT EXIST "%cd%/../assets/shaders/" mkdir "%cd%/../assets/shaders/"
IF NOT EXIST "%cd%/intermediate/" mkdir "%cd%/intermediate/"

@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (tools\\glslc.exe -g "%%I" -o "./intermediate/%%~nI%%~xI.spv")
@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (tools\\spirv-cross.exe "./intermediate/%%~nI%%~xI.spv" --output "../assets/shaders/%%~nI%%~xI.json" --reflect)
@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (tools\\spirv-opt.exe -O "./intermediate/%%~nI%%~xI.spv" -o "../assets/shaders/%%~nI%%~xI.spv")
pause