# Deutsch-Jozsa Algorithm (2-qubit version)
# Determines if function is constant or balanced

QUBITS 2

# Initialize qubit 1 to |1>
X 1

# Apply Hadamard to both qubits
H 0
H 1

# Oracle (example: balanced function f(x) = x)
CNOT 0 1

# Apply Hadamard to qubit 0
H 0

# Measure qubit 0
# Result: |0> = constant, |1> = balanced
MEASURE 0
