## How-to
### Compile
After fetching the repository, do

    cd install
    source envset.sh
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install ..
    make -j12 install
    cd ../install
    source envset.sh

### Analyzing
After installing and locating EdgeEffect_NUM_1.root
                              EdgeEffect_NUM_2.root
                              EdgeEffect_NUM_3.root
                              EdgeEffect_NUM_4.root in install dir, do

    cd install
    ./bin/analysis <1~4>

1 : (87,0) <> (87,1)

2 : (87,0) <> (88,0)

3 : (0,0) <> (0,1)

4 : (0,0) <> (1,0)

### Further Analyzing
Edit analysis.cc after line #127 !
