Prerequisites
=============

Custom version of NS-3 and specified version of ndnSIM needs to be installed.

The code should also work with the latest version of ndnSIM, but it is not guaranteed.

    # Checkout latest version of ndnSIM
    mdkir persistent-interests
    cd persistent-interests
    git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3
    git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
    git clone --recursive https://github.com/named-data-ndnSIM/ndnSIM.git ns-3/src/ndnSIM

    # Set correct version for ndnSIM and compile it
    cd ns-3
    git checkout 2c66f4c
    cd src/ndnSIM/
    git checkout a9d889b
    cd NFD/
    git checkout 85d60eb
    cd ../ndn-cxx/
    git checkout 787be41
    cd ../../../
    ./waf configure -d optimized
    ./waf
    sudo ./waf install

    # Checkout persistent-interest scenario
    cd ..
    git clone git@gitlab.itec.aau.at:philipp-moll/PI-scenario.git
    cd PI-scenario

    # Patch ndnSIM, NFD and ndn-cxx
    cp extern/tag-host.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/interest.* ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/data.* ../ns-3/src/ndnSIM/ndn-cxx/src/
    cp extern/tlv.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/encoding/
    cp extern/qci.hpp ../ns-3/src/ndnSIM/ndn-cxx/src/encoding/

    cp extern/forwarder.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/
    cp extern/pit* ../ns-3/src/ndnSIM/NFD/daemon/table/
    cp extern/retx-suppression-exponential.cpp ../ns-3/src/ndnSIM/NFD/daemon/fw/

    cp extern/ndn-producer.hpp ../ns-3/src/ndnSIM/apps/
    cp extern/*tracer* ../ns-3/src/ndnSIM/utils/tracers/

    cp extern/queue.h ../ns-3/src/network/utils/

    # Recompile ndnSIM
    cd ../ns-3/
    ./waf
    sudo ./waf install

    # Compile and run scenario
    cd ../PI-scenario
    ./waf configure
    ./waf
    ./waf --run=push-simple --vis

Compiling
=========

To configure in optimized mode without logging **(default)**:

    ./waf configure

To configure in optimized mode with scenario logging enabled (logging in NS-3 and ndnSIM modules will still be disabled,
but you can see output from NS_LOG* calls from your scenarios and extensions):

    ./waf configure --logging

To configure in debug mode with all logging enabled

    ./waf configure --debug

If you have installed NS-3 in a non-standard location, you may need to set up ``PKG_CONFIG_PATH`` variable.

Running
=======

Normally, you can run scenarios either directly

    ./build/<scenario_name>

or using waf

    ./waf --run <scenario_name>

If NS-3 is installed in a non-standard location, on some platforms (e.g., Linux) you need to specify ``LD_LIBRARY_PATH`` variable:

    LD_LIBRARY_PATH=/usr/local/lib ./build/<scenario_name>

or

    LD_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name>

To run scenario using debugger, use the following command:

    gdb --args ./build/<scenario_name>


Running with visualizer
-----------------------

There are several tricks to run scenarios in visualizer.  Before you can do it, you need to set up environment variables for python to find visualizer module.  The easiest way to do it using the following commands:

    cd ns-dev/ns-3
    ./waf shell

After these command, you will have complete environment to run the vizualizer.

The following will run scenario with visualizer:

    ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib ./waf --run <scenario_name> --vis

If you want to request automatic node placement, set up additional environment variable:

    NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

or

    PKG_LIBRARY_PATH=/usr/local/lib NS_VIS_ASSIGN=1 ./waf --run <scenario_name> --vis

Available simulations
=====================

<Scenario Name>
---------------

Description
