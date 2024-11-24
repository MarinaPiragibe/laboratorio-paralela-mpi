# Generate UUID
UUID := $(shell uuidgen)

# Grouped target to compile all binaries
compile-all: compile-first compile-segundo compile-sequencial

# Compile targets
compile-first:
	mpicc -o prompt-inicial prompt-inicial.c

compile-segundo:
	mpicc -o prompt-modificado prompt-modificado.c

compile-sequencial:
	mpicc -o sequencial sequencial.c

run-all: prompt-inicial prompt-modificado sequencial

# Run binaries with MPI
prompt-inicial: compile-first
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt 
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

prompt-modificado: compile-segundo
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

sequencial: compile-sequencial
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt

clean:
	rm -rf prompt-inicial prompt-modificado sequencial runs/*
