# android's dalvik vm running on macOS!
The purpose of this project is purely for entertainment. It involves adapting the Dalvik virtual machine from Android 1.6 to 64-bit and making adjustments to various APIs and dependency libraries for macOS, allowing this ancient version of the DEX virtual machine to run on the latest macOS.

Since this project only adapts a basic while-loop-based simulation for interpretation (without threaded, mterp/nterp, or JIT execution), it theoretically supports both ARM64 and x86_64 devices. I’ve only tested it on ARM64 macOS.

# how to run?

```dalvik -cp hello.dex com.dexvm.test.Main```

# how to build?
this is a very simple cmake project, u should know how to build. brew install all the deps before building or run. 

dep list:
1. zlib
2. libffi