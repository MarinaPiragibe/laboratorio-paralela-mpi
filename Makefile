# Grouped target to compile all binaries
compile-all: compile-first compile-segundo compile-sequencial

# Compile targets
compile-first:
	mpicc -o primeiro-prompt primeiro-prompt.c

compile-segundo:
	mpicc -o segundo-prompt segundo-prompt.c

compile-sequencial:
	mpicc -o sequencial sequencial.c

# Run binaries with MPI
primeiro-prompt: compile-first
	mpirun -np 1 ./$@
	mpirun -np 2 ./$@
	mpirun -np 4 ./$@
	mpirun -oversubscribe -np 8 ./$@

segundo-prompt: compile-segundo
	mpirun -np 1 ./$@
	mpirun -np 2 ./$@
	mpirun -np 4 ./$@
	mpirun -oversubscribe -np 8 ./$@

sequencial: compile-sequencial
	mpirun -np 1 ./$@

clean:
	rm -f primeiro-prompt segundo-prompt sequencial
