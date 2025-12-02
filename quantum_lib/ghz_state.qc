# GHZ State (Greenberger-Horne-Zeilinger)
# Creates maximally entangled state of 3 qubits
# |GHZ> = (|000> + |111>) / sqrt(2)

QUBITS 3

# Create superposition on first qubit
H 0

# Entangle all qubits
CNOT 0 1
CNOT 0 2

# Measure all - should get 000 or 111 with equal probability
MEASURE 0
MEASURE 1
MEASURE 2
