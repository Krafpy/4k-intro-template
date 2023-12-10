# 4k intro template

This is an example minimal template for building 4kB intros on Windows
(but can apply to larger ones as well). It is highly inspired by
[Inigo Quilez's (iq) templates](https://madethisthing.com/iq/Demo-Framework-4k)
for 4k shader-based intros, but with a more explicit
writing and lots of comments so that the code appears more user-friendly to
beginners. This project requires doesn't rely on Visual Studio (only the
Build Tools) to demonstrate the details of the compilation.
The rendering and music generation is mostly based on iq's example,
but the synthetizer is custom made.

For complete beginners to demoscene, check out this article on how to
[*Teach Yourself Demoscene in 14 Days*](https://github.com/psenough/teach_yourself_demoscene_in_14_days).

This template itself contains a very simple example intro that
can be built and run directly. Of course, when designing a real demo, you will want adapt,
inline, rewrite or remove some of the code here in addition to writing your own
(possibly in assembly).

## File structure

- `main.c`: entrypoint, creates the window and starts the music and rendering loop;
- `config.h`: global settings;
- `glext.h`, `khrplatform.h`: self-contained interfaces of OpenGL functions;
- `fp.h`: useful set of approximate floats ([by iq](https://iquilezles.org/articles/float4k/));
- `intro.h`/`intro.c`: rendering initialisation and update;
- `shader.inl`: the shaders' sources;
- `synth.h`/`synth.c`: functions for the audio synthetizer;
- `music.h`/`music.c`: music file generation.

## Build

The compilation tricks to generate a small executable follow the recommandations described
in [this article by iq](https://iquilezles.org/articles/compilingsmall/), with some updates,
and are detailed in the `build.ps1` script.

Building the project requires the following tools to be installed and accessible via a
PowerShell command line:

- Microsoft Visual C++ compiler `cl.exe` and linker `link.exe`
(e.g. using the Powershell Developer Command Prompt). These are bundled in the the
[Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/?q=build+tools+for+visual+studio)
(note that you don't need to install Visual Studio itself).
- [`crinkler`](https://github.com/runestubbe/Crinkler) (e.g. in PATH)

If you are using an antivirus software (Windows Defender included),
it is likely to detect the compiled executable as
a [trojan](https://en.wikipedia.org/wiki/Trojan_horse_(computing)).
You'll need to whitelist the file or the entire folder in your antivirus before compiling or running it.

Build an uncompressed debug version (uses MSVC's linker):

```powershell
.\build.ps1
```

or the compressed release (uses crinkler):

```powershell
.\build.ps1 -release
```

and run:

```powershell
.\main.exe
```

To see all the build options enter:

```powershell
.\build.ps1 -help
```