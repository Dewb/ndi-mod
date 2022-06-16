ln -s ~/norns dep/norns
mkdir -pv build && cd build
cmake ..
make install-dust
