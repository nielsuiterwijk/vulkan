IF NOT EXIST "%cd%/../bin/shaders/" mkdir "%cd%/../bin/shaders/"

@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (%VULKAN_SDK%/Bin32/glslangValidator.exe -V "%%I" -o "../bin/shaders/%%~nI%%~xI.spv")
@for %%I IN (*.vert; *.tesc; *.tese; *.geom; *.frag; *.comp) DO (%VULKAN_SDK%/Bin32/glslangValidator.exe -V "%%I" -o "../bin/shaders/%%~nI%%~xI.spv")
pause