# Quantum Fourier Transform (3-qubit)
# Performs QFT on 3 qubits

QUBITS 3

# Example: Initialize state |101>
X 0
X 2

# QFT Circuit
H 0
S 0
T 0

H 1
S 1

H 2

# Swap qubits (for correct ordering)
# Note: SWAP not implemented, using CNOT sequence
CNOT 0 2
CNOT 2 0
CNOT 0 2

# Measure all qubits
MEASURE 0
MEASURE 1
MEASURE 2
