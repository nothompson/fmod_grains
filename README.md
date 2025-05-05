# Live Granular Plugin for FMOD Studio  (2.03)

Open source C++ custom dsp for FMOD. Integrate granularity in your sounds and background music based on parameters from game engines like Unity or Unreal. Polyphonic grains with a maximum of 16 grains per trigger. Density, length, and playbackrate/pitch parameters, easily modulated or automated in FMOD Studio.

## Download Guide
1. Download FMOD Studio [here](https://www.fmod.com/ "FMOD Homepage")
2. Download FMOD Core API [here](https://www.fmod.com/core "FMOD Core")
3. Become familiar with each of their file paths. For example, mine looks like this:
C:\Program Files\FMOD SoundSystem\FMOD Studio 2.03.07
and
C:\Users\*yourname*\fmod\fmodSetup\FMODStudioAPIWindows
4. Git pull or manually download the github files to a folder:
C:\Users\*yourname*\fmod\fmodSetup\FMODStudioAPIWindows\api\core\examples\vs2019
Here there are many other visual studio files for custom fmod dsps.
5. Among the visual studio files in the folder are the main cpp file **"fmod_grains.cpp"** and **"grain_header.h",** these need to be moved to a specfic path for fmod to compile and build the plugin to a dll.
6. Move the fmod_grains.cpp to the plugin examples in the core directory: C:\Users\*yourname*\fmod\fmodSetup\ *FMODStudioAPIWindows\api\core\examples\plugins*
7. Move the grain_header.h to the core include directory C:\Users\*yourname*\fmod\fmodSetup\ *FMODStudioAPIWindows\api\core\inc*
