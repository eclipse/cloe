.PHONY: all clean
all: build
	cmake --build build

build:
	cmake -E make_directory build
	conan install --build=missing --install-folder build -g cmake .
	cmake -E chdir build cmake -Wdev ..

clean:
	-rm -r build
