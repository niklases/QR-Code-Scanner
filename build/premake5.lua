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

project "QRCode"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   files { "../src/Header/**.h", "../src/Source/**.cpp" }
   
   -- WINDOWS
   filter "system:windows"
      includedirs { "../include/opencv/build/include" }
      libdirs { "../include/opencv/build/x64/vc16/lib", "../include/opencv/build/x64/vc16/bin" }
      filter {"system:windows", "Debug"}
         links { "opencv_world4100d.lib" }
      filter {"system:windows", "Release"}
         links { "opencv_world4100.lib" }
   
   -- LINUX  
   filter "system:linux"
      includedirs { "/usr/local/include/opencv4"}
      libdirs { "/usr/local/lib" }
      links {"opencv_stitching", "opencv_highgui", "opencv_ml", "opencv_videoio", "opencv_video", 
             "opencv_objdetect", "opencv_calib3d", "opencv_imgcodecs", "opencv_features2d",
             "opencv_dnn", "opencv_flann", "opencv_photo", "opencv_imgproc", "opencv_core"}
   
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
	
   filter "system:windows"
      filter {"system:windows", "Debug"}
   	   postbuildcommands { "copy $(SolutionDir)..\\include\\opencv\\build\\x64\\vc16\\bin\\opencv_world4100d.dll   $(SolutionDir)bin\\Debug\\opencv_world4100d.dll" }
      filter {"system:windows", "Release"}
   	   postbuildcommands { "copy $(SolutionDir)..\\include\\opencv\\build\\x64\\vc16\\bin\\opencv_world4100.dll   $(SolutionDir)bin\\Release\\opencv_world4100.dll" }
