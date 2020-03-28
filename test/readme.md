### NOTE

Tests are currently supported only on `Ubuntu`. It may run on other Linux versions.

### Running tests

1. Install `ArduinoJson` by Benoit Blanchor

2. Install `Google Test` framework:

```
   sudo apt-get install libgtest-dev
   sudo apt-get install cmake
   cd /usr/src/gtest
   sudo cmake CMakeLists.txt
   sudo make
   sudo cp \*.a /usr/lib
```

3. Update `ARDUINO_LIB_ROOT` in `CMakeList.txt` to point to your local Arduino library location

4. Build and run:

```
   cmake CMakeList.txt
   make
   ./executeTests
```
