.. _gettingstarted-label:

Getting Started
===============

Requirements
------------

====================    ================   ====
Package                 Version            Note
====================    ================   ====
**gcc**                 >=4.9.0            If you want to build llvm with gcc too, please follow llvm's `requirement <http://llvm.org/docs/GettingStarted.html#software>`_ for gcc.
**clang**               >=3.8.0            Tested with various versions up to v6.0.0 .
**llvm**                >=3.8.0            Tested with various versions up to v6.0.0 .
**cmake**               >=3.1.0
**Python**              >=3.0.0
**Gurobi Optimizer**    v7.0.2 or v7.5.1   Not required, but it is the main supported optimizer. We provide a SCIP implementation as well, but SCIP is much slower. Only free with academic license, but there is free trial.
====================    ================   ====

.. _installation-label:

Installation
------------

1. Download the source code package from download page
2. Create a directory with your preferred name ``<PROJ_DIR>``
3. Extract the source code package in ``<PROJ_DIR>`` directory
4. Extract Gurobi library package in ``<PROJ_DIR>``
5. Go to `Gurobi website <http://www.gurobi.com/index>`_ to register and request a license, also download the Gurobi library
6. In bin subdirectory of Gurobi directory run ``./grbgetkey <key-you-obtained>`` to activate your Gurobi

.. NOTE::
    * If you are building llvm from source, make sure you put the build or install directory in your ``$PATH`` environment variable.
    * There is a `known <https://bugs.llvm.org/show_bug.cgi?id=23352>`_ `problem <https://bugs.launchpad.net/ubuntu/+source/llvm/+bug/1387011>`_
      with the llvm-dev packages on many Ubuntu versions, where CMake fails to find parts of LLVM.
      Building from source will work, or you can manually change some files in /usr. See :ref:`install-troubleshoot-label`

7. $ ``cd <PROJ_DIR>\cgra-me-uoft && make``

.. SEEALSO::

    Reference the :ref:`cmake-build-label` guide if you want to use the complete feature, this is only a Makefile wrapper to CMake.

See :ref:`install-troubleshoot-label` if you encountered any problem in installation process.

Environment Setup
-----------------

There is a script ``cgra-me-uoft/cgrame_env`` that when *run* (not intended to be ``source``'d) will
drop you into another instance of your ``$SHELL`` with
several convenient and essential environment variables set, including:

* ``PATH`` will be augmented so that the most important
  CGRA-ME binaries and scripts can be invoked with out explicit paths.
* ``CGRA_MAPPER`` will have the name of the mapper executable,
  with the expectation that it will be resolved against the above modified path.
* Several other variables related to file locations.

Running Benchmarks
------------------

With the above installation and environment setup complete,
you can now try out the software on some of the included benchmarks and architectures (covered here)
and then move on to making your own (covered elsewhere).

Bulk Benchmark Runs
^^^^^^^^^^^^^^^^^^^

The script ``cgra-me-uoft/benchmarks/experiment_runner`` was created to make this fairly easy.
It is decently configurable and may be the simplest way to run experiments when using a single computer.
Configuration is done via a python file (specified by ``-f``, though it has a default) and
it can run experiments in parallel (``-j`` option).
For more documentation, and specifically documentation about the configuration input,
we direct you to run it with the ``--help`` option.

There is a configuration file example included as a starting point,
so if you just want to run the software, simply:

1. $ ``cd  cgra-me-uoft/benchmarks``
2. $ ``./experiment_runner``

.. NOTE::
    * This will take a while to finish, especially if parallelism is not enabled

Individual Benchmark Running
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``experiment_runner`` script is (of course) capable of running exactly one experiment,
but it is useful to know some details if you plan on making your own,
or want to get to know the software better.

Generate A DFG
""""""""""""""

The first step is to generate a DFG (Data Flow Graph).
A DFG is a representation of a computation kernel that captures the logical movement of data from inputs though operations to outputs.
We'll use one of the included benchmarks as an example.

1. $ ``cd cgra-me-uoft/benchmarks/microbench/sum``
2. $ ``make clean    # start from scratch``
3. $ ``ls            # see what files are present before``
4. $ ``make          # compile with clang, optimize, and emit the DFG``
5. $ ``ls            # note graph_loop.dot was created``

This will generate several files, the most important of which is ``graph_loop.dot`` which holds the DFG
that resulted from the processing of the loop in the C file.
The format used is the classic 'dot' format and so can be viewed with several programs, for example ``xdot``.
More details about how this step works can be found at :ref:`dfg-generation-label`.

.. NOTE::
    If LLVM was not installed, a pre-generated set of DFGs translated from the C benchmarks are distributed with the framework.
    They can be found within the individual benchmark directories as ``cgra-me-uoft/benchmarks/microbench/*/pre-gen-*.dot``.
    To use them with ``experiment_runner``, simply ``cp pre-gen-graph_loop.dot graph_loop.dot`` in each experiment directory you would like to use.

Mapping
"""""""

This is the process of taking a DFG and a CGRA architecture and
finally mapping the computation kernel onto the CGRA.
We will use one of the included architectures for demonstration.
After building the benchmark, now run this:

1. $ ``"$CGRA_MAPPER" --II 2 -g graph_loop.dot --xml "$CGRA_ME_ROOTDIR/arch/simple/archfiles/adres-no-torroid.xml"``

If you looked into the ``experiment_runner``'s help message, or investigate the Makefiles, you may wish to know that an equivalent way to run this would be to do:

1. $ ``make run_mapper "CGRA_ARCH_ARGS=--xml $CGRA_ME_ROOTDIR/arch/simple/archfiles/adres-no-torroid.xml" 'CGRA_MAPPER_ARGS=--II 2'``

Now if everything is setup correctly, you should see the mapper running, and it should complete in less than a minute.
We are mapping a benchmark that does sum over an array,
we are using an architecture similar to ADRES,
the architecture is using our default parameters of a four-by-four functional unit array,
and we are specifying that there are two contexts to context-switch between.

As a side note, this will use our default Integer Linear Programming-based mapping technique,
as opposed to our Simulated Annealing based one --
a choice which can be overridden with the ``--mapper`` option.
Run ``$CGRA_MAPPER --help`` for more details.

Visualization
-------------

After a successful mapping, the ``$CGRA_ME_ROOTDIR/output`` directory is created.
Opening the ``CGRA.html`` document will show visualization of the MRRG nodes within the CGRA as well as the mapped nodes.
The top navigation bar shows tabs for multiple contexts of the CGRA.
Yellow nodes represent unused resources.
Teal nodes represent inputs or outputs of cross-context elements such as registers.
Blue nodes are composite nodes in the hierarchy.
Double-clicking on these nodes descends into the hierarchy.
Selecting and right-clicking on any node within the expanded hierarchy will collapse the node back into the composite node.
Red nodes represent used resources. Hovering over these nodes will inform the user within a tool-tip the part of the mapped DFG used that resource.

Conclusion
----------

This is only a getting started guide to CGRA-ME, there are many other features, you can read the :ref:`userguide-label` if you want to know more about them.

.. _install-troubleshoot-label:

Troubleshooting
---------------

I have cmake error about cannot find llvm package
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Using apt-get: to fix the `known <https://bugs.llvm.org/show_bug.cgi?id=23352>`_ `problem <https://bugs.launchpad.net/ubuntu/+source/llvm/+bug/1387011>`_,
  we have provided some ``.cmake`` files in ``cgra-me-uoft/doc/ubuntu-llvm-fix``. **Intended for 16.04.**
  Copy these files to to ``/usr/share/llvm-3.8/cmake`` and make this symbolic link:

    ``ln -s /usr/lib/x86_64-linux-gnu/libLLVM-3.8.so.1 /usr/lib/llvm-3.8/lib/libLLVM-3.8.so.1``.

  Inspired by `a stack overflow answer <https://stackoverflow.com/a/44311146/2256231>`_.
* Using apt-get: the package manager could mess up stuff sometimes, please check if you have any of the following directory in you ``$PATH``: ``/usr/`` or ``/usr/bin/`` or ``/usr/sbin/``.
  If you don't see any of the above add ``/usr/`` to your ``$PATH``, then try build again
* Build from llvm source: try to add ``<LLVM_DIR>/build/`` to your ``$PATH``, then try build again

.. NOTE::
    If the above did not help you, it might be that you have a existing llvm installed and it is tricking the package searcher from cmake. Reference the user guide and run cmake with the argument ``-DCMAKE_PREFIX_PATH=<path/to/your/llvm>`` to override the default llvm cmake file.

Send us an e-mail if you have any other problem!
