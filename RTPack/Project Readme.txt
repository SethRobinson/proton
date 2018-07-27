RTPack

A console mode app that only runs in text mode.

RTPack.exe is a utility created for Proton SDK.  It's required to do the following:

- Compress any file into an .rtpack format
- Created .rtfont files (a custom font format with kerning and "Seth Ansi Color" that include the font data as well as the font images directly in the file)
- Create .rttex files, a flexible format for images
- If compiled with the optional PVRTC support it can create many PVRTC formats as well, this is only useful if you're on tight memory constraints on mobile, was a big deal in 2013, not so much now

Setup to compile on:

* Windows
* Linux - do "sh linux_compile.sh" from the /linux dir on a linux computer to compile.  Then do "./RTPack" to run from the bin dir.

Check protonsdk.com for help and tutorials.
