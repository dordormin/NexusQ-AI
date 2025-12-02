# Bell State Circuit Test
# Creates an entangled Bell state |00> + |11>

QUBITS 2

# Create superposition on qubit 0
H 0

# Entangle qubit 0 and 1
CNOT 0 1

# Measure both qubits
M 0
M 1
