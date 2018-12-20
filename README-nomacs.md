# Building exiv2 for nomacs

## Build exiv2 (Windows)

### Compile dependencies

- [Qt](https://www.qt.io/)

### Compile LibQPSD

- copy `CMakeUserPathsGit.cmake` and rename it to `CMakeUserPaths.cmake`
- add your library paths to the `${CMAKE_PREFIX_PATH}` in `CMakeUserPaths.cmake`
- Open CMake GUI
- set this folder to `where is the source code`
- choose a build folder (e.g. `build2017-x64`)
- Hit `Configure`then `Generate`
- Open the Project
- Compile the Solution (build Release and Debug)
- Build `INSTALL`
- You should now have a `qpsd.dll` in $YOUR_QT_PATH$/plugins/imageformats
- nomacs will automatically copy the plugins from there to it's build folder
