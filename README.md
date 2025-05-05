# Live Granular Plugin for FMOD Studio  (2.03)

Open source C++ custom dsp for FMOD. Integrate granularity in your sounds and background music based on parameters from game engines like Unity or Unreal. Polyphonic grains with a maximum of 16 grains per trigger. Density, length, and playbackrate/pitch parameters, easily modulated or automated in FMOD Studio.

![alt text](https://imgur.com/N133GlU)

## Download Guide
1. Download FMOD Studio [here](https://www.fmod.com/ "FMOD Homepage")
2. Download FMOD Core API [here](https://www.fmod.com/core "FMOD Core")
3. Become familiar with each of their file paths. For example, mine looks like this:
C:\Program Files\FMOD SoundSystem\FMOD Studio 2.03.07
and
C:\Users\ *yourname*\fmod\fmodSetup\FMODStudioAPIWindows
4. Git pull or manually download the github files to a folder:
C:\Users\ *yourname*\fmod\fmodSetup\FMODStudioAPIWindows\api\core\examples\vs2019
Here there are many other visual studio files for custom fmod dsps.
5. Among the visual studio files in the folder are the main cpp file **"fmod_grains.cpp"** and **"grain_header.h",** these need to be moved to a specfic path for fmod to compile and build the plugin to a dll.
6. Move the fmod_grains.cpp to the plugin examples in the core directory: C:\Users\ *yourname*\fmod\fmodSetup\ *FMODStudioAPIWindows\api\core\examples\plugins*
7. Move the grain_header.h to the core include directory C:\Users\ *yourname*\fmod\fmodSetup\ *FMODStudioAPIWindows\api\core\inc*
8. Open the Visual Studio solution project from the github, it should open up both the grain header and grains.cpp files.
9. **Make sure you have your Visual Studio on Release x64**
10. Now right click on fmod_grains on the right in the solution explorer tab, and click build.
11. It should give a message saying build successful and a dll should be accesible in the examples folder: FMODStudioAPIWindows\api\core\examples\vs2019\_builds\fmod_grains\Release\x64\fmod_grains64.dll
12. Now navigate back to your fmod studio folder, and access C:\Program Files\FMOD SoundSystem\FMOD Studio 2.03.07\plugins. Here you can place the dll, and youre all set!
13. Open FMOD Studio, open an effect plugin, navigate to plugin effects at the bottom, and you should see the granular plugin ready to go.
