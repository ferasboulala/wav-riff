# wav-riff
WAV Metadata C++ library 

Example of usage is found in `wavFileTest.cpp`.

A wave file (`.wav` extension) is a sound file that follows a format called RIFF. With this library, you can read and write any type of RIFF compliant chunk to wave file (including data). Simply define a chunk if it does not exist yet (RIFF header, fmt, fact, bext and cart chunks are created by default) and read/write to the file.

The purpose of this library is to be able to modify a wave file without losing chunks like every other programs/libraries do because the norm specifies to ignore a chunk if it is not recognized. Instead, it declares it as undefined and it lets the user the choice of dropping or holding undefined chunks. If one wishes to add his own chunk, it is possible by simply adding chunks to the `WavData` object and read/writing.
