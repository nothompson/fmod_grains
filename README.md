# NTGrains : Live Granular Plugin for FMOD Studio  (2.03)

Open source C++ custom dsp for FMOD. Integrate granularity in your sounds and background music based on parameters from game engines like Unity or Unreal. Polyphonic grains with a maximum of 16 grains and a buffer freeze. Per grain density, length, and playbackrate/pitch parameters, easily modulated or automated in FMOD Studio.

![alt text](https://i.imgur.com/kOl4Kpf.png)

## Download Guide
1. Go to the [FMOD website](https://www.fmod.com/ "FMOD Homepage") and sign in/sign up
2. Download FMOD Studio and FMOD Engine [here](https://www.fmod.com/download "download page")
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

## Unity Integration Guide
1. To ensure you can use this plugin inside of a unity project, you need to ensure the dll file is placed in the project
2. Follow this [FMOD tutorial]("https://www.fmod.com/docs/2.00/unity/integration-tutorial.html") if you're just getting started 
3. Otherwise you can just navigate to the project folders in Unity at the bottom and go to Assets/Plugins/FMOD/platforms/win/lib/x86_64, and copy and paste the plugin dll from C:\Program Files\FMOD SoundSystem\FMOD Studio 2.03.07\plugins.
4. Once thats done you should be all set to start automating granular effects from game parameters! 
