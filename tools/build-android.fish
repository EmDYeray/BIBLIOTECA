#!/usr/bin/env fish
set -l root (realpath (dirname (status --current-filename))/..)
set -l sdk /opt/android-sdk
if set -q ANDROID_SDK_ROOT
    set sdk $ANDROID_SDK_ROOT
end
set -l sdl "$HOME/.cache/starry-android/SDL2"
set -l deps "$HOME/.cache/starry-android/deps"
set -l project "$root/android"

for needed in "$sdl/android-project/gradlew" "$sdl/android-project/gradle/wrapper/gradle-wrapper.jar" "$deps/x64-android/lib/pkgconfig/cairo.pc" "$deps/arm64-android/lib/pkgconfig/cairo.pc" "$sdk/ndk/30.0.16248370/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++"
    if not test -e "$needed"
        echo "Missing Android dependency: $needed" >&2
        exit 1
    end
end

mkdir -p "$project/app/src/main/java/org" "$project/gradle/wrapper" "$root/dist"
cp -R "$sdl/android-project/app/src/main/java/org/libsdl" "$project/app/src/main/java/org/"
cp "$sdl/android-project/gradlew" "$project/gradlew"
cp "$sdl/android-project/gradle/wrapper/gradle-wrapper.jar" "$project/gradle/wrapper/gradle-wrapper.jar"
printf 'sdk.dir=%s\ncmake.dir=/usr\n' "$sdk" > "$project/local.properties"

if not set -q JAVA_HOME
    set -gx JAVA_HOME /usr/lib/jvm/java-17-openjdk
end
set -gx ANDROID_HOME "$sdk"
set -gx ANDROID_SDK_ROOT "$sdk"
set -gx ANDROID_NDK_HOME "$sdk/ndk/30.0.16248370"
set -gx STAR_ANDROID_SDL2 "$sdl"
set -gx STAR_ANDROID_DEPS "$deps"
"$project/gradlew" --project-dir "$project" --no-daemon :app:assembleDebug
or exit $status

set -l apk "$project/app/build/outputs/apk/debug/app-debug.apk"
cp "$apk" "$root/dist/Starry-Library-Android-universal-debug.apk"
echo "APK: $root/dist/Starry-Library-Android-universal-debug.apk"
