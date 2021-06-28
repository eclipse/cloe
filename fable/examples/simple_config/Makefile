.PHONY: all clean
all: build
	cmake -E cmake --build build

build:
	cmake -E make_directory build
	cmake -E chdir build cmake -E cmake -Wdev ..

clean:
	-rm -r build
