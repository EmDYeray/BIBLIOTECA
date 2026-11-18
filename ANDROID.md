# Android local build

This is a separate Android 10+ build of the existing C++ library. It targets
`x86_64` for local tests in Waydroid (Android API 33); the Linux and Windows
builds remain independent. It does not publish anything to GitHub.

## Dependencies

- Android SDK platform 35, build tools 35 and NDK `30.0.16248370`, at
  `/opt/android-sdk` or the location in `ANDROID_SDK_ROOT`.
- JDK 17 in `/usr/lib/jvm/java-17-openjdk`, CMake, Ninja, pkg-config and fish.
- SDL2 source release `release-2.32.10` checked out in
  `$HOME/.cache/starry-android/SDL2`.
- vcpkg `de36db04550a8625e3154b1a758c60035901f244`, with
  `cairo:x64-android` and `fontconfig:x64-android` installed to
  `$HOME/.cache/starry-android/deps`.

## Build

From the repository root:

```fish
fish tools/build-android.fish
```

The script copies SDL2's Android Java bridge and Gradle wrapper into ignored
build paths, writes `android/local.properties`, then builds a debug APK with
Gradle 8.9 and Android Gradle Plugin 8.7.3. It outputs
`dist/Starry-Library-Android-x86_64-debug.apk`. The debug APK uses Gradle's
local test key, so it can be installed for a Waydroid smoke test; it is not a
release package.

The activity extracts bundled assets into the app's private directory before
starting the native SDL2 thread. The renderer loads the existing desktop
shaders as OpenGL ES 3.0 source and converts Cairo's BGRA image pixels into
RGBA for GLES textures. The first launch may take longer while assets are
copied. Any change to bundled assets requires raising `versionCode` in
`android/app/build.gradle` and the corresponding marker in `LibraryActivity`.

## Touch controls

The four direction buttons at the lower left move within the room. Drag a
finger across the right half of the scene to rotate the camera. Tap a book to
walk up and read it; tap the catalog, lectern or room switch at the top to use
those sections. When reading, use the bottom arrows to turn pages and the top
button to close. SDL2 maps a finger tap to a mouse click for those buttons.

## Waydroid test

After the container is running, install the debug APK with Waydroid's local
installer, open the app and inspect `waydroid logcat` for `AndroidRuntime`,
`SDL`, `libc`, and `Starry Library` diagnostics if startup fails. Waydroid's
optional ARM translation is not involved: the APK contains native x86_64 code.
