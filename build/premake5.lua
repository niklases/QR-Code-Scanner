-- premake5.lua
workspace "QRCode"
   configurations { "Debug", "Release" }
   platforms { "Static64", "Shared64" }
   
  filter "platforms:Static32"
    kind "StaticLib"
    architecture "x32"

  filter "platforms:Static64"
    kind "StaticLib"
    architecture "x64"

  filter "platforms:Shared32"
    kind "SharedLib"
    architecture "x32"

  filter "platforms:Shared64"
    kind "SharedLib"
    architecture "x64"

project "HelloWorld"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   files { "../src/Header/**.h", "../src/Source/**.cpp" }
   
   includedirs { "../include/opencv/build/include" }
 
   libdirs { "../include/opencv/build/x64/vc16/lib", "../include/opencv/build/x64/vc16/bin" }
   
   filter "Debug"
      links { "opencv_world4100d.lib" }
   filter "Release"
      links { "opencv_world4100.lib" }
   
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
	  
   filter "Debug"
   	   postbuildcommands { "copy $(SolutionDir)..\\include\\opencv\\build\\x64\\vc16\\bin\\opencv_world4100d.dll   $(SolutionDir)bin\\Debug\\opencv_world4100d.dll" }
	   
   filter "Release"
   	   postbuildcommands { "copy $(SolutionDir)..\\include\\opencv\\build\\x64\\vc16\\bin\\opencv_world4100.dll   $(SolutionDir)bin\\Release\\opencv_world4100.dll" }
