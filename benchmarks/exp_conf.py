import itertools;
import os;

def configuration(environment):
	cgrame_root = environment['cgrame_root'];

	config =  {
		'benchdirs' : [
			os.path.join(cgrame_root, 'benchmarks'),
		],
		'archdirs' : [
			# os.path.join(cgrame_root, 'arch/simple'), # adl v0
			os.path.join(cgrame_root, 'arch/simple/archfiles'), # adl v1
		],
		'xml_arch_bases' : [
			('hetorth_x', 'arch-hetero-orth.xml',),
			('arch-homo-diag.xml',) ,
		],
		'cpp_arch_bases' : [
			('simorth_pp',  '0',),
			('simdiag_pp',  '1',),
			('adress_pp',   '2',),
		],
		'arch_args' : [
			('2x2', 'cols=2 rows=2'),
			('4x4', 'cols=4 rows=4'),
			('6x6', 'cols=6 rows=6'),
			('lat1', 'fu_latency=2')
		],
		'arch_specs' : [
			('simorth_4x4', 'simorth_pp', ['4x4']),
			('sd 4x4', 'simdiag_pp', ['4x4']),
			('sd 6x6', 'simdiag_pp', ['6x6']),
			('adres 4x4', 'adress_pp', ['4x4']),
			('hetorth', 'hetorth_x'),
			('homo-diag', 'arch-homo-diag.xml'),
		],
		'bench' : [
			('sum','microbench/sum',),
			('nomem1', 'microbench/nomem1',),
			('accumulate', 'microbench/accumulate'),
			('mac', 'microbench/mac'),
		],
		'crga_mapper_args' : [
			('args-default', ''),
			('args-scpi', '--mapper-opts \'ILPMapper.ilp_solver=scip\''),
			('args-II_is_2', '--II 2'),
		],
		'parse_directves' : [
			('solver', '\s*([\w ]+) Solver Specified', '\\1'),
			('presolve time', 'Presolv(e|ing) [tT]ime: (.+)', '\\2'),
			('mapper timeout', 'MapperTimeout: (.+)', '\\1')
		],
		'experiments' : [
			*itertools.product(
				['homo-diag', 'adres 4x4', ],
				['sum', 'nomem1', 'accumulate', 'mac', ],
				['args-default', 'args-II_is_2'],
			),
		],
	}

	for bad_ex in itertools.product(
		['homo-diag'],
		['accumulate','mac'],
		['args-default', 'args-II_is_2'],
	):
		config['experiments'].remove(bad_ex);

	return config;
