#### Use this program to run the old nokia or blackberry java games on your computer

- First install SDL via your package manager e.g **Fedora**
`sudo dnf install SDL2-devel`

- Make the build folder on the project root and run the Makefile
`mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make`

- Run the binary
`./j2me-ui`
