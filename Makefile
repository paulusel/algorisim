config:
	cmake -S . -B ./build -G Ninja
compile:
	cmake --build ./build
clear:
	rm -rf build
all:
	$(MAKE) clear
	$(MAKE) config
	$(MAKE) compile
