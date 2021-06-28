all:
	mkdir -p build
	conan install --build=missing --install-folder=build conanfile.txt
	cd build && cmake ..
	make -C build

clean:
	rm -rf build
