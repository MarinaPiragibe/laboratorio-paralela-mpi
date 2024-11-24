# Generate UUID
UUID := $(shell uuidgen)

# Grouped target to compile all binaries
compile-all: compile-first compile-segundo compile-sequencial

# Compile targets
compile-first:
	mpicc -o primeiro-prompt primeiro-prompt.c

compile-segundo:
	mpicc -o segundo-prompt segundo-prompt.c

compile-sequencial:
	mpicc -o sequencial sequencial.c

run-all: primeiro-prompt segundo-prompt sequencial

# Run binaries with MPI
primeiro-prompt: compile-first
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

segundo-prompt: compile-segundo
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt
	mpirun -np 2 ./$@  > runs/$@-2-$(UUID).txt
	mpirun -np 4 ./$@  > runs/$@-4-$(UUID).txt
	mpirun -np 8 ./$@  > runs/$@-8-$(UUID).txt

sequencial: compile-sequencial
	mpirun -np 1 ./$@  > runs/$@-1-$(UUID).txt

clean:
	rm -rf primeiro-prompt segundo-prompt sequencial runs/*
