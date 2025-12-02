# Superposition Test Circuit
# Creates equal superposition of all 3-qubit basis states

QUBITS 3

# Apply Hadamard to create superposition on all qubits
H 0
H 1
H 2

# Measure all
MEASURE 0
MEASURE 1
MEASURE 2
