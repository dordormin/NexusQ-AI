# Quantum Random Number Generator
# Generates truly random bits using quantum superposition

QUBITS 4

# Create superposition on all qubits
H 0
H 1
H 2
H 3

# Measure all - each has 50% chance of 0 or 1
MEASURE 0
MEASURE 1
MEASURE 2
MEASURE 3

# Result: 4 truly random bits (0-15 in decimal)
