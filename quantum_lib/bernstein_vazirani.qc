# Bernstein-Vazirani Algorithm
# Finds hidden bit string in one query

QUBITS 3

# Initialize ancilla to |1>
X 2

# Apply Hadamard to all qubits
H 0
H 1
H 2

# Oracle encoding secret string s = "11"
CNOT 0 2
CNOT 1 2

# Apply Hadamard to query qubits
H 0
H 1

# Measure to reveal secret string
MEASURE 0
MEASURE 1
