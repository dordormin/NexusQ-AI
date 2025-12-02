# W State (3-qubit)
# Creates W state: |W> = (|100> + |010> + |001>) / sqrt(3)
# Different entanglement properties than GHZ

QUBITS 3

# Approximate W state construction
H 0
H 1
H 2

# Apply controlled operations
CNOT 0 1
CNOT 1 2
X 0

# Measure
MEASURE 0
MEASURE 1
MEASURE 2
