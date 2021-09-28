.. _userguide-label:

User Guide
==========

.. _cmake-build-label:

CMake Build
-----------

The build method in the :ref:`gettingstarted-label` guide uses Makefile. The Makefile used is just a wrapper to the internal CMake. A more advanced user can use CMake directly to build the project.

Set-up as the :ref:`installation-label` guide step one to step six suggested.
`$ cd build` and run `$ cmake <your-options> ..`

CMake Possible Options
^^^^^^^^^^^^^^^^^^^^^^

``-DBUILD_LLVM_PASSES=<value>``
    ``<value>`` can either be ``ON`` or ``OFF`` to either enable or disable the building of the llvm loop extraction pass.

.. SEEALSO::
    Refer to `CMake's website <https://cmake.org/cmake/help/latest/manual/cmake.1.html>`_ for CMake options, also reference `CMake's website on variables <https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html>`_ for CMake variables that you want to modify.

.. _dfg-generation-label:

Loop DFG Generation
-------------------

The user can create their own benchmarks that they want to map to different CGRA architectures. Here is the way of doing that.

Generate User Custom DFG
^^^^^^^^^^^^^^^^^^^^^^^^

1. Create a new directory in benchmark/userbench directory with the same name as the name of your benchmark source file.

2. Create a new file with the name Makefile in the new directory you created.

3. :ref:`Modify <source-code-requirement-label>` your source code and include the :ref:`dfg-loop-tag-label` in the loop of your interest.

.. NOTE::
    There are limitations to what kind of loop can generate DFG and be mapped. Limitations are explained in :ref:`dfg-generation-limitation-label`.

4. Add necessary :ref:`dfg-generation-options-label`, including compiler flags and flags to indicate which loop to generate the DFG for.

5. Put line: `include ../rules.mk` at the end of the Makefile and save the file.

6. Run ``$ make`` in the current directory and the DFG with be generated.

.. _dfg-generation-options-label:

DFG Generation Options
^^^^^^^^^^^^^^^^^^^^^^

Put the following DFG Generation options in the beginning of your Makefile to configure it to generate DFG.

``LOOP_TAGS=<tags>``
    ``<tags>`` are your loop tags for the loop that you want to generate DFG for. (If you want to generate a DFG for more than one loop, separate the loop tags by space.

``DFG_FLAGS=<flags>``
    There are two possible options for ``<flags>``, they are ``-no-phi-removal`` to prevent phi node removal and ``-no-leaf-removal`` to prevent the removal of useless leaf nodes (including comparison nodes). These two flags are both not enabled by default to generate mappable DFGs.

``CFLAGS=<flags>``
    The ``<flags>`` will be directly passed to clang compiler. Reference to `Clang documentation <https://clang.llvm.org/docs/ClangCommandLineReference.html>`_ for possible flags.

.. _source-code-requirement-label:

Source Code Requirement
^^^^^^^^^^^^^^^^^^^^^^^^

Some modifications might be required for the DFG generation process to take place correctly. Before modifying any code, you have to have a clear understanding of which functions should be inlined and which functions should not be inlined.

1. This DFG generation software can only support single C source file with header files. Make sure every thing that you want to map to a single source file.

2. All the functions that contains within your loop of interest should be inlined. You must add ``__attribute__((always_inline))`` to the functions and put the definitions in the corresponding header file.

3. All the functions that contains your loop of interest should not be inlined. You must add ``__attribute__((noinline))`` to the functions and put the definitions in the source file.

.. _dfg-loop-tag-label:

DFG Loop Tag
^^^^^^^^^^^^

For all the loops that you want to generate DFGs for, you have to add special tags to them.

In the first line of the loop, add ``//DFGLOOP: <tag>`` where ``<tag>`` is an arbitrary name you given to the loop, which will be referenced use to reference in :ref:`dfg-generation-options-label`. Please look at any of the microbench examples if you are not sure about how it is done.



XML User Guide
==============

The XML language provides a platform for CGRA architects to create relatively simple designs quickly and gives an easy way to visualize the architecture you are creating.


Language Format
---------------

The following is a high level description of each tags functionality

- <cgra>
	- This tag begins the document. You must open and close the document with this tag.
- <definition ..../>
	- Allows you to create variables. Similar to the #definition tag in c++
- <module name="...">
	- A module can be thought of as a black box you can refer to by name. It contains inputs, outputs and bidirectionals that are connected to a series of processing elements on the inside.
- <architecture>
	- The architecture is the physical layout of all of the modules. Here you describe the position of each module and how each module connects to each other module


Generic Document Structure
--------------------------


.. code-block:: xml

	<cgra>

		<definition..../>

		<module name="...">
			<!-- stuff -->
		</module>

		<architecture rows="n" cols="m">
			<!-- stuff -->
		</architecture>
	</cgra>


Creating a Module
-----------------


Naming a module
^^^^^^^^^^^^^^^

- When you create a *block* in the architecture, you must fill it with a module with a name. How to do this is found in the *Describing the Architecture* section. The following code describes how to assign a name to a module that can be used later inside the architecture.

.. code-block:: xml

    <module name="genericname">
        <!-- module contents -->
    </module>

Adding inputs/outputs/inouts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Each module must have inputs, outputs, or bidirectionals (inouts) so that it is able to connect to surrounding modules in the architecture. Each input/output/inout has a name so it can be referred to later on in both the module and architecture. The following code describes how to add input and output ports.

.. code-block:: xml

    <module name="genericname">
        <input name="input"/>
        <output name="output"/>
        <inout name="inout"/>
        <!-- more stuff -->
    </module>

- These ports can be referred to within the architecture by this.*port name*


Adding an instance
^^^^^^^^^^^^^^^^^^

- Inside each module, you can create instances of different processing elements, each with its own set of inputs and outputs. You can refer to a PEs inputs and outputs using a name which you assign to each PE. The currently supported PEs with their corresponding XML arguments are:

    - FuncUnit
        - 'op' refers to the operations supported by the unit separated by whitespace (more info can be found in the c++ user guide on this). This defaults to add sub
        - 'size' refers to the bit width of the operands. This defaults to 32
    - MEMUnit
        - none
    - Register
        - 'size' refers to the bit width of the register This defaults to 32
    - Multiplexer
        - 'ninput' refers to the number of inputs
        - 'size' refers to the bit width of the inputs. This defaults to 32
    - Tristate
        - 'size' refers to the bit width of the inputs. This defaults to 32
    - IO
        - 'size' refers to the bit width of the inputs. This defaults to 32
    - RegisterFile
        - 'ninput' refers to the number of input ports to the register file
        - 'noutput' refers to the number of output ports from the register file
        - 'log2-nregister' refers to the log base 2 of the number of items the refister can hold
        - 'size' refers to the bit width of each item. This defaults to 32

- The inputs and outputs of each PE can be found in the C++ documentation

- The following is an example of how to add an instance to your architecture

.. code-block:: xml
 
    <module name="genericname">
        <inst module="FuncUnit" name="func" op="add sub mul"/>
    </module>



- A description of the functionality of each PE can be found in the C++ API documentation

Adding a wire
^^^^^^^^^^^^^

- A wire is equivalent to a port. It provides an intermediary step for connections.

.. code-block:: xml

    <module name="genericname">
        <input name="input"/>
        <output name="output"/>
        <inout name="inout"/>

        <wire name="wire"/>
        <!-- more stuff -->
    </module>



Adding a connection
^^^^^^^^^^^^^^^^^^^

- Connections can be made between ports. The connection must have an attribute describing where it is coming from and where it is going. The currently supported attributes are:
    - to - connects some input(s) to one output
    - from - connect one input to some output(s)
    - select-from - takes multiple inputs as arguments and muxes them to one output
    - distribute-to- distributes 1 input to many outputs (no muxes)

.. code-block:: xml

    <module name="genericname">
        <input name="input"/>
        <output name="output"/>
        <inout name="inout"/>

        <wire name="wire"/>
        <connection from="this.input" to="wire"/>
        <connection from="wire" to="this.output"/>
        <!-- more stuff -->
    </module>

Describing the Architecture
---------------------------

Patterns
^^^^^^^^

- Patterns allow the user to easily create a repeated tile layout on the grid
- The **row-range** and **col-range** attributes define which rows and columns to operate on. The **row** and **col** attributes define how many blocks are occupied by 1 instance of the pattern (they default to 1 if left out)
- The current pattern position iterates from left to right, and moves down to the next row when there are no more columns over which to iterate in the described area
- The language currently does not support creating connections with **row** and **col** attributes not equal to 1

Creating a pattern without the **row** and **col** attributes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- The following code is an example of creating a pattern for which each position on the grid is filled with a block containing a module
- Here, I have created a 3x3 grid and am operating on all positions (counting starts at 0 for row-range and col-range)

.. code-block:: xml

    <architecture rows="3" cols="3">
        <pattern row-range="0 2" col-range="0 2">
            <block module="module"/>
        </pattern>
    </architecture>

Creating a pattern with the **row** and **col** attributes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- When you use these attributes, you are creating a set of blocks that will be repeatedly stamped out. Each of these blocks takes up space on the grid. When you declare the blocks inside the pattern, you add blocks from left to right, top to bottom in the space defined by the pattern.
- The following example is a creation of a pattern using the **row** and **col** attributes. 

.. code-block:: xml

    <architecture rows="8" cols="8">
        <pattern row-range="0 7" col-range="0 7" row="2" col="2">
            <block module="A"/> <block module="B"/>
            <!-- row break -->
            <block module="C"/> <block module="D"/>
        </pattern>
    </architecture>

Counters
^^^^^^^^

- In this language, there are 3 types of counters: **counter, row-counter, col-counter**
- Each counter starts at 0 and increases by 1 each time you move to the next instance of a pattern **note: double check this**
- The counter
    - Each time you move to the next instance of a pattern stamp, this counter will increment by 1. It never resets to 0
- The row-counter
    - This counter starts at 0 and increments by 1 every time you move to a new row within the pattern
- The col-counter
    - This counter starts at 0 and increments every time you move to the next column in a pattern. If the current grid position moves to a new row, this counter gets reset to 0
- The following is an example of how to use these counters

.. code-block:: xml

    <architecture rows="3" cols="3">
        <pattern row-range="0 2" col-range="0 2" counter="i">
            <connection from="(rel 0 0).portname" to="(rel 0 -(i)).portname"/>
        </pattern>
    </architecture>

- NOTE: When using the counter, you must give it a name. Here, I have given it the name 'i'. Ensure that the name of the counter is just 1 character.
- When you would like to use the counter to represent a number, you must enclose it in brackets. For example, if the value of the counter named 'i' is 5, (i) will become 5 in the code above.

Wrap-Around
^^^^^^^^^^^

- The language supports the ability to 'wrap-around'. This means that if I create a pattern that will be created between columns 0 to 2, and I refer to a column that is outside of these bounds, the language will use the modulus operator to refer to a block that lies within the pattern instead if this feature is activated. Here is an example with wrap-around ON

.. code-block:: xml

    <architecture rows="3" cols="3">
        <pattern row-range="0 2" col-range="0 2" wrap-around="on">
            <connection from="(rel 0 0).portname" to="(rel 0 3).portname"/>
        </pattern>
    </architecture>

- In this example, if the current working grid position is 0, 0, (rel 0 3) will refer to 0, 0. This is because normally, (rel 0 3) would refer to block 0, 3 and this block is outside of the pattern range. Instead of referring to block 0, 3, the language treats this as block 0, 1 because it has 'wrapped around' the current pattern.
- This feature defaults to OFF

Inter-Block connections
^^^^^^^^^^^^^^^^^^^^^^^

- Connections can be made between blocks in the architecture with patterns. You can refer to the block to which you would like to connect not by its absolute position on the grid, but by its relative position to the current position on the grid. An example of such a connection is as follows:

.. code-block:: xml

    <connection from="(rel a b)"/>

- Here, **a** and **b** are placeholders for integers. **a** represents the row offset (downward is positive) and **b** the column offset (to the right is positive). For example, (rel 0 0) refers to the current grid position, and (rel 1 1) refers to a position 1 down and 1 to the right of the current grid position.
- The following is an example of a pattern that connects each block to the block to its right
- To refer to a blocks port (input/output/inout), use the following format:
    - blockname.portname

.. code-block:: xml

    <architecture rows="8" cols="8">
        <pattern row-range="0 7" col-range="0 6">
            <connection from="(rel 0 1).portname" to="(rel 0 0).portname"/>
        </pattern>
    </architecture>

Referring to individual blocks


.. code-block:: xml

    <connection from="block_a_b_.portname" to="(rel 0 0).portname"/>

- Here, **a** represents the row and **b** represents the column of the desired block (counting starts at 1)

Syntactic Sugar
^^^^^^^^^^^^^^^

Some designs for a CGRA are quite common, so as a part of the language we have added syntactic sugar to make these common designs easy to represent. These common designs are listed below...

**NSWE Mesh:** On the grid, this architecture consists of IOs along every side of the grid (but not in the corners), and the interior entirely filled with PEs. Each PE connects to each other via north, south, east and west connections. For example, the east input/output of one PE will be connected to the west output/input of the PE to its east, the north input/output of the PE will be connected to the south output/input of the PE to its north, etc. The following code is an example of how to create such an architecture:


.. code-block:: xml

    <architecture col="4" row="4" cgra-cols="2" cgra-rows="2">
	    <mesh out-north=".out"
	          out-east=".out"
	          out-west=".out"
	          out-south=".out"
	          in-north=".in0"
	          in-east=".in1"
	          in-west=".in2"
	          in-south=".in3"
	          io="every-side-port"
	          >
	      <interior row="1">
	        <block module="block1" mode="add sub mul div and or xor shl shr sshr"/>
	      </interior>
	    </mesh>
	  </architecture>

You must include the out-north, etc attributes in the mesh tag to determine which ports in each PE you would like to use for the north, south, west and east ports.


There are a few things to note about this functionality that haven't been mentioned. When using the *mesh* tag, you must include the *cgra-cols* and *cgra-rows* attributes to the architecture. This determines how many rows and columns to use for the PEs in the mesh, and how many to leave for the IOs. Generally, leave 2 rows and 2 columns for the IOs.

The second thing to note is the *interior* tag. This tag functions similarly to *pattern*. It determines which block(s) to fill the interior of the cgra design with. You can use the **row** and **col** attributes here in the same way as described above.


**NSWE + Diagonal Mesh:** This is almost exactly the same as the NSWE Mesh, except this includes the northeast, northwest, southeast, southwest connections (note: there are still no IOs in the corners of the grid). An example of how to create such an architecture is below.


.. code-block:: xml

    <architecture col="4" row="4" cgra-cols="2" cgra-rows="2">
    <diagonal out-north=".out"
              out-east=".out"
              out-west=".out"
              out-south=".out"
              out-northeast=".out"
              out-northwest=".out"
              out-southeast=".out"
              out-southwest=".out"
              in-north=".in0"
              in-east=".in1"
              in-west=".in2"
              in-south=".in3"
              in-northeast=".in4"
              in-northwest=".in5"
              in-southeast=".in6"
              in-southwest=".in7"
              io="every-side-port"
              >
      <interior row="1">
        <block module="block1" mode="add sub mul div and or xor shl shr sshr"/>
      </interior>
    </diagonal>
  </architecture>

Note that instead of using the *mesh* tag, the *diagonal* tag is used instead. Everything else is consistent with *mesh*.

Examples
--------


- Creating a NSWE Mesh of functional units with syntactic sugar

.. code-block:: xml

	<cgra>
	  <module name="block1">
	    <input name="in0"/> <input name="in1"/> <input name="in2"/> <input name="in3"/>
	    <output name="out"/>
	    <inst name="func" module="FuncUnit" op="add sub mul div and or xor shl shr"/>
	    <inst name="register" module="Register"/>
	    <wire name="in_a"/> <wire name="in_b"/> <wire name="func_out"/>
	    <connection select-from="this.in0 this.in1 this.in2 this.in3 register.out" to="in_a in_b"/>
	    <connection from="in_a" to="func.in_a"/>
	    <connection from="in_b" to="func.in_b"/>
	    <connection select-from="in_a in_b func.out" to="func_out"/>
	    <connection from="func_out" to="register.in"/>
	    <connection select-from="func_out register.out" to="this.out"/>
	  </module>
	  <architecture col="4" row="4" cgra-cols="2" cgra-rows="2">
	    <mesh out-north=".out"
	          out-east=".out"
	          out-west=".out"
	          out-south=".out"
	          in-north=".in0"
	          in-east=".in1"
	          in-west=".in2"
	          in-south=".in3"
	          io="every-side-port"
	          >
	      <interior row="1">
	        <block module="block1"/>
	      </interior>
	    </mesh>
	  </architecture>
	</cgra>

C++ API User Guide
==================

The C++ API provides a flexible platform for CGRA architects to create designs in c++ and determine whether or not certain benchmarks can be mapped to them.

Before you begin coding
-----------------------
Before you write any code, you must include the correct header files. These are as follows:

- <CGRA/Module.h>
- <CGRA/ModuleComposites.h>
- <CGRA/CGRA.h>
- <CGRA/ILPMapper.h>
- <CGRA/OpGraph.h>

Include any other files you require


Basic Overview
--------------

When using the C++ API, there is an order of operations to follow which I will outline below.

1. Create the CGRA pointer.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The mapper takes in the pointer as an argument. Before you pass the pointer to the mapper, you must 'fill' it with the architecture

2. Adding primitives.
^^^^^^^^^^^^^^^^^^^^^

Here you add the different PEs which will make up the architecture.

3. Adding connections.
^^^^^^^^^^^^^^^^^^^^^^

Here, you connect the PEs together. Connections are made between ports and each PE has its own ports with a naming convention, which will be described later.



Adding the CGRA Pointer
-----------------------

To add the CGRA pointer to your file, you use the following syntax:

.. code-block:: c++

    std::unique_ptr<CGRA> tempName(new CGRA());

Here, tempName is the name you would like to give to your pointer. This name will be used for the rest of the document.



Adding Processing Elements
--------------------------

Here is a list of the PEs that are available to add to your architecture:

**FuncUnit:** This is a functional unit that performs LLVM computations. You can give it the ability to perform multiple operations of your choice. An extensive list of operations that it can perform is below. To add one to your architecture, use the following syntax (the types of each variable are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new FuncUnit(std::string name, std::vector<OpGraphOpCode> supported_modes = {OPGRAPH_OP_ADD, OPGRAPH_OP_MUL}, unsigned size = DEFAULT_SIZE));

Be sure that no 2 functional units have the same name. The supported modes default to add and multiply. A full list can be found below. The size is the bit width of the operands and result.

Ports: 
    - 'in_a' is an input
    - 'in_b' is an input
    - 'out' is an output

**MEMUnit (needs testing):** The MEMUnit is a functional unit that only supports store and load operations. It can be called using the following syntax:

.. code-block:: c++

    tempName->addSubModule(new MEMUnit(std::string name, unsigned size = DEFAULT_SIZE));

The name and size arguments represent the same thing as they do in the FuncUnit.

Ports:
    - 'addr' is the address to read or write to
    - 'data_in' is the input data
    - 'data_out' is the output data

**Register:** To add a register to your architecture, use the following syntax (variable types are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new Register(std::string, unsigned size = DEFAULT_SIZE));

Ports:
    - 'in' is the input data
    - 'out' is the output data

**Multiplexer:** The multiplexer selects a single input as output from a choice of many inputs. It can be called using the following syntax (types are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new Multiplexer(std::string, unsigned mux_size, unsigned size = DEFAULT_SIZE));

Be sure that each mux has a unique name. The mux_size determines how many inputs the mux can take. The size is the bit width of the operands and defaults to 32.

Ports:
    - 'inN' where N is replaced by the number of the input port you are refering to (counting starts at 0)
    - 'out' is the output of the mux
    - 'select' decides which input to take

**RegisterFile:** To add a register file to your architecture, use the following syntax (the variable types are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new RegisterFile(std::string name, unsigned NumInputPorts, unsigned NumOutputPorts, unsigned Log2Registers, unsigned size = DEFAULT_SIZE));

Ports:
    - 'inN' refers to the Nth input port
    - 'address_inN' refers to the Nth input address
    - 'WEN' refers to the Nth write enable port
    - 'outN' refers to the Nth output port
    - 'address_outN' refers to the Nth port for output address

**TriState:** To add a new TriState to your architecture, use the following syntax (variable types are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new TriState(std::string, unsigned size = DEFAULT_SIZE));

**IO:** IO stands for input/output. It allows the CGRA to interact with external devices. It can be added to your architecture with the following syntax (types are included for clarity):

.. code-block:: c++

    tempName->addSubModule(new IO(std::string, unsigned size = DEFAULT_SIZE));

Ports:
    - 'in' is the input port
    - 'out' is the output port
    - 'bidir' is the inout port

Be sure the name of each IO is unique. The size is the bit width of the operands and defaults to 32.


Here is a list of operations that the FuncUnit can perform:

**nop** - no operation

**trunc** - truncate

**sext** - sign extend

**phi** - phi instruction

**add** - add operands

**sub** - subtract operands

**mul** - multiply operands

**div** - divide operands

**and** - and operands

**or** - or operands

**xor** - xor operands

**shl** - shift operand to left by some number

**shra** - arithmetic shift right 

**shrl** - locigal shift right 

**gep** - get external pointer

**icmp** - return boolean based on comparison of operands

**load** - read from memory

**store** - write to memory

Adding Connections
------------------

Each PE you add to the architecture has its own port names. The naming conventions are as follows:

When you connect PEs together in the architecture, you must make connections by port names. The following is the syntax for creating connections between PEs


Creating Your Own Processing Element
------------------------------------

In the C++ API, we allow the user to create a "black box" with user defined functionality and naming conventions. The user is able to create their own processing element that can consist of a combination of the provided processing elements. The way this is done is as follows:

1. Create a class. This class must inherit from Module.

2. The constructor for your class is where the creation of the custom module happens. You must pass the "name" string to the Module constructor when you create your constructor. Other than that, you have the option to pass the "size" as a parameter (which is an integer that represents the bit width of the operands), but if left out it defaults to 32.

3. Add the modules to your design using the following syntax:

.. code-block:: c++

    tempName->addSubModule(new yourModule(yourArgumens));


4. Add the ports to your design. Your design should include input/output ports so that multiple instances of this design can be connected together later in the architecture. Your design can also include internal ports, which can be used as an intermediate when connecting PEs together. To add a port to your design, use the following syntax inside the constructor of your module:

.. code-block:: c++

    addPort("portName", portType, size);

If you want to refer to the port you have just created inside the constructor, you can type *this.portName*. *portType* is a variable of type port_type and can be any of the following: PORT_INPUT, PORT)OUTPUT, PORT_OUTPUT_REG, PORT_BIDIR, PORT_UNSPECIFIED. The size is the bit width of the port and defaults to 32 if left out.

5. Add the internal connections. The connections between instances of your module will be made later in the architecture definition. Here, you must connect the PEs that exist within your module using the syntax outlined in the above sections.

Once you have created your module, be sure to include its header file into your main architecture file. The following is an example of how to add an instance of your custom module to your code:

.. code-block:: c++

    tempName->addSubModule(new yourModule(yourArgumens));

For an example of the C++ API being used, look at the ``arch_mem_fu.cpp`` file within src/archs. In there, you will find constructors for custom modules that use the C++ API.

