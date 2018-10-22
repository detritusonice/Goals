# Goals
A toy command line goal tracker

This is a small project developed using TDD principles by a lone hobbyist programmer.
Its objective is to let me use techniques I learned-or try to, in a practical setting.
One can compile and link on Linux using make:
<pre>
  <code>make goals</code>        creates the executable for this toy app.
  <code>make testgoals</code>    creates the testsuite executable.
</pre>
You will need the googleTest framework:
<pre><code>#get latest stable source from github saved usually as googletest-master.zip
#select an appropriate parent folder to place the zip into,
# in case you desire to keep the source in your machine

unzip googletest-master.zip
cd googletest-master</code>
cmake -DBUILD_SHARED_LIBS=OFF
make
cmake -DBUILD_SHARED_LIBS=ON
make

cd googlemock
sudo cp -a libgmock*.a /usr/lib
sudo cp -a libgmock*.so /usr/lib

cd gtest
sudo cp -a libgtest*.a /usr/lib
sudo cp -a libgtest*.so /usr/lib

cd ../include
sudo cp -a gtest /usr/include

#remove googletest-master directory if you wish...
cd ../..
rm -rf googletest-master
</code></pre>
