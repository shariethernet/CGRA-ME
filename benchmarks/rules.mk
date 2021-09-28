#remove all implicit rules
MAKEFLAGS += -r

BENCHNAME ?= $(shell basename `pwd`)

CFLAGS ?= -O3

OPT_DISABLE_FLAGS ?= -fno-vectorize -fno-slp-vectorize -fno-unroll-loops

LLVM_DFG_PLUGIN ?= ../../../build/lib/libDFG.so
LOOP_PARSER ?= ../../../build/script/LoopParser.py

LOOP_TAGS ?= loop

DFG_FLAGS += -loop-tags '$(LOOP_TAGS)'

DFG_TARGETS = $(patsubst %,graph_%.dot,$(LOOP_TAGS))

build: $(DFG_TARGETS)

run: run_mapper

## FOR BUILDING ##

$(DFG_TARGETS): $(BENCHNAME).bc $(BENCHNAME).tag
	opt '$(BENCHNAME).bc' -o '/dev/null' -load '$(LLVM_DFG_PLUGIN)' --dfg-out -in-tag-pairs '$(BENCHNAME).tag' $(DFG_FLAGS)

%.bc: %.tagged.c
	clang -emit-llvm -c '$*.tagged.c' -o '$*.bc' $(CFLAGS) $(OPT_DISABLE_FLAGS)

%.ll: %.bc
	llvm-dis '$*.bc'

%.tagged.c %.tag: %.c
	'$(LOOP_PARSER)' '$*.c' '$*.tagged.c' '$*.tag'

clean:
	rm -f '$(BENCHNAME).bc' '$(BENCHNAME).ll' $(DFG_TARGETS) '$(BENCHNAME).tag' '$(BENCHNAME).tagged.c'

## FOR RUNNING ##

prepare_run: build

run_mapper:
	'$(CGRA_MAPPER)' $(CGRA_MAPPER_ARGS) $(CGRA_ARCH_ARGS) -g $(firstword $(DFG_TARGETS))
