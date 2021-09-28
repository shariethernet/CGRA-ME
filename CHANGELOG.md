# Changelog
All notable changes to CGRA-ME will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
- Fixes to ADL parser
- Updated Verilog generation to templating system

## [1.0.0] - 2018-03-26
### Added
- Mapping Visualization by @tech4me
- SCIP added as a second option for the ILP solver over Gurobi
- Mapper options for both the Gurobi and SCIP solvers can now also be controlled from the ``mapper_config.ini`` configuration file
- New version of the ADL parser front-end and is now the default parser
- Env script that drops the user into a subshell with the correct libraries in the path

### Changed
- Main executable is now ``cgra-me``
- Executable options now have standard names and have been reordered
- Large refactor of all mapper code including the Annealer and ILP formulations and it's now much more understandable and modular

### Fixed
- Bitstream generation fixed to work with new architectures
