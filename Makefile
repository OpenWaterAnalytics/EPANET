# Remove before upstreaming to OWA/EPANET's repository
# Only for dev convenience

all:
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DLUA_SCRIPTING=1 && cmake --build .

test:
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DLUA_SCRIPTING=1 -DBUILD_TESTS=ON && cmake --build . && cd ..
	cd build/tests && ctest -C Release --output-on-failure

clean:
	rm -rf build

.PHONY: all clean test