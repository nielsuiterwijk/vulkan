IF NOT EXIST "%cd%/../assets/shaders/" mkdir "%cd%/../assets/shaders/"

@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (%VULKAN_SDK%/Bin32/glslangValidator.exe -V "%%I" -o "../assets/shaders/%%~nI%%~xI.spv")
pause