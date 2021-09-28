.. _limitation-label:

Limitation
==========

There are some limitation to the software, this might vary between different versions.

.. _dfg-generation-limitation-label:

DFG Generation Limitation
-------------------------

1. If you have nested loops, only the innermost loop will be looked at. So, if you tagged the outer loop, it will not generate DFG for you.

2. You can only have one basic block within the loop body, meaning you can not have any branching in your loop body. (If statement, switch statement)

3. You must follow the to make all of your functions are properly inlined or not-inlined, otherwise there is undefined behaviour.

4. DFG generation has been having issues with structs, this has been noticed and will be fixed.

Mapper Limitation
-----------------

1. The mapper currently only generates the datapath portion of the mapping.

2. DFGs with cycle (caused by back-edges) that have a length larger than one will not be mapped correctly. (Fixed in upcoming release)

3. The mapper will not work correctly if the DFG provided has imbalanced path within. (Fixed in upcoming release)

4. There is behaviour differences between the ILP Mapper and the Simulated Annealing Mapper, the result from ILP should be the prefered one.

5. The Simulate Annealing Mapper needs a slower schedule for DFGs that have back-edges.

