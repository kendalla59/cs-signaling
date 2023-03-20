# cs-signaling
## Case study for a railroad signaling system

This is a **CMake** project, originally implemented
on a Linux system. If you don't have cmake already
installed, try (on debian, e.g. Ubuntu):

`sudo apt install cmake`

To set up the makefiles for this project, assuming
you are in the home directory of cs-signaling:

`cmake -S . -B build`

If everything went as expected, you can now build
and run the application:

```
cd build
make
./cs_signaling
```
