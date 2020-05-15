
# build settings is in file 'Matmakefile'


all: matmake/matmake
	@echo using Matmake buildsystem
	@echo for more options use 'matmake/matmake -h'
	@matmake/matmake

debug:
	@matmake/matmake config=debug

clean:
	matmake/matmake clean

matmake/matmake:
	matmake -C
