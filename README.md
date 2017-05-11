PrimU
=====

PrimU is a prototype emulator for the HP Prime calculator based on [Unicorn Engine](https://github.com/unicorn-engine/unicorn).

The program is currently so early in development that there are esentially no features implemented. 
PrimU is currently targeting HP Prime Firmware version 20130808 as it contains the most debugging information.


Compiling
---------
Visual Studio 2015 is needed for compilation.

Just open `PrimU.sln` and build.

Running
-------
In order to run PrimU one must first extract the `armfir.elf` file from the 201308080 firmware update for the calculator.
This can be done by mounting the FAT-16 filesystem prestent at an 8kb offset inside the APPDISK.DAT file from the firmware update.
See [this](https://tiplanet.org/hpwiki/index.php?title=HP_Prime/Firmware_files) wiki for more information.

Once the file is extracted one can run PrimU with the following command:

    PrimU.exe [path to armfir.elf]

License
-------

This project is released under the [GPL license](COPYING).