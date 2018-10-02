IF NOT EXIST "%cd%/../assets/shaders/" mkdir "%cd%/../assets/shaders/"

@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (%VULKAN_SDK%/Bin32/glslangValidator.exe -V "%%I" -o "../assets/shaders/%%~nI%%~xI.spv")
@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (SPIRV-Cross.exe "../assets/shaders/%%~nI%%~xI.spv" --output "../assets/shaders/%%~nI%%~xI.json" --reflect)
pause