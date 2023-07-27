# Hand-off: Evaluating mice and optical hand trackers in typing-dominant environments

This is the software that drives the user study for the paper
*Hand-off: Evaluating mice and optical hand trackers in typing-dominant environments* (working title).
Hopefully this thing will get published soon/at all...

## Notes for whoever wants to run this

* A computer with two monitors is highly, highly recommended.
* This software has only been tested on 1080p monitors,
  so 1080p or greater resolution monitors are recommended.
* To figure out which monitor the browser needs to be placed on,
  run `.\handGestureUserStudy --configure-mouse`.
  This will place your mouse on the correct monitor for the browser.
* Each participant needs an ID number. This is just the number of participants that have been run through.
* Participant ID is important: it is used for counterbalancing and writing the log files.
* There are some files/directories of interest that are created
  in the **same working directory as the executable**:
    * `ids.lock` => Keeps track of which user study IDs have been used and which haven't.
      This is to make sure that log files don't get accidentally overwritten.
    * `Logs/userX.log` => The log file for user X. This contains the collected data for later analysis.
    * `HTMLTemplates/` => HTML templates for rendering the user study pages.
      This is automatically emitted by the build system.
    * `www/` => Root directory for static files for the user study pages.
      This is automatically emitted by the build system.
    * `LeapC.dll` => Runtime for the Leap Motion C API. This is automatically emitted by the build system.
* If you are using multiple computers to run this study, make sure to copy the current `ids.lock`
  to the computer you are currently using.
* Make sure to run the executable in the same directory as all of the above files.
* Make sure all log files get back to me somehow.
* If something goes wrong during the user study, make a note of the user ID that errored,
  delete the corresponding log file, and re-run the user study using a new ID.
* The user study is done in the browser at [**http**://localhost:5000](http://localhost:5000).
* Consent forms, pre-surveys, and post-surveys will be done with pen and paper.

## Building

### Build requirements

The LeapC SDK only works on Windows at the current moment,
so this program is only compatible with Windows.
This project also requires C++20 and CMake.
The 64-bit MSVC compiler bundled with Visual Studio 2022 was used during development,
but any C++20 compliant MSVC should work.

### Gathering Dependencies

This project requires the following dependencies:
* LeapC
* raylib
* cpp-httplib
* rapidjson

raylib, cpp-httplib, and rapidjson are included as submodules of this repository.
**LeapC must be procured from elsewhere** due to licensing restrictions.
To include LeapC in the project, find the LeapC SDK, download it,
and place it in a folder named `LeapSDK` in the root of this repository.
If you're not sure if you got the right files, the directory structure should look something like...
```
LeapSDK/
    include/
    lib/
    samples/
    LICENSE.md
    README.md
    ThirdPartyNotices.md
```

## Building

After gathering the dependencies, run the following commands in the root of the repository.
```powershell
mkdir build
cd build
cmake.exe -G "Visual Studio 17 2022" -S.. -B.
cmake.exe --build . --config Debug --target handGestureUserStudy
```

If necessary, replace the generator with the appropriate one for your Visual Studio configuration.