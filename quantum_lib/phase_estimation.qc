# Quantum Phase Estimation (Simplified)
# Estimates the phase of an eigenvalue

QUBITS 3

# Initialize target qubit to eigenstate
X 2

# Apply Hadamard to counting qubits
H 0
H 1

# Controlled unitary operations (simplified)
# U^(2^0) controlled by qubit 1
CNOT 1 2
T 2

# U^(2^1) controlled by qubit 0
CNOT 0 2
S 2
CNOT 0 2

# Inverse QFT on counting qubits
H 1
H 0

# Measure counting qubits
MEASURE 0
MEASURE 1
