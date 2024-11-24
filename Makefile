# Generate UUID
UUID := $(shell uuidgen)

# Grouped target to compile all binaries
compile-all: compile-first compile-segundo compile-sequencial

# Compile targets
compile-prompt-inicial:
	mpicc -o prompt-inicial prompt-inicial.c

compile-prompt-modificado:
	mpicc -o prompt-modificado prompt-modificado.c

compile-com-scatter:
	mpicc -o prompt-com-scatter prompt-com-scatter.c
	
compile-sequencial:
	mpicc -o sequencial sequencial.c

run-all: prompt-inicial prompt-modificado prompt-com-scatter sequencial

# Run binaries with MPI
prompt-inicial: compile-prompt-inicial
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt 
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

prompt-modificado: compile-prompt-modificado
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

prompt-com-scatter: compile-com-scatter
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

sequencial: compile-sequencial
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt

clean:
	rm -rf prompt-inicial prompt-modificado sequencial runs/*
