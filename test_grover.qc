# Grover's Algorithm (2-qubit simplified demo)
# Search for |11> state

QUBITS 2

# Initialize superposition
H 0
H 1

# Oracle: Mark |11> state
Z 0
Z 1

# Diffusion operator (simplified)
H 0
H 1
X 0
X 1
CNOT 0 1
X 0
X 1
H 0
H 1

# Measure
MEASURE 0
MEASURE 1
