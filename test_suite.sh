#!/bin/bash
# NexusQ-AI Master Test Suite
# Automates the verification of Kernel, Shell, Network, and AI features.

echo "=== NexusQ-AI Test Suite ==="

# 1. Build Everything
echo "[TEST] Building System..."
cmake . > /dev/null && make nexus_shell app_demo qnetd > /dev/null
if [ $? -ne 0 ]; then
    echo "[FAIL] Build Failed."
    exit 1
fi
echo "[PASS] Build Successful."

# 2. Run Shell Commands via Input Redirection
echo "[TEST] Testing Shell & Kernel Features..."

# Start Node 2 (Receiver) in background
./qnetd 2 > qnetd2_output.log 2>&1 &
QNETD2_PID=$!
echo "[TEST] Started Node 2 (PID $QNETD2_PID)"

# Start Node 3 (Destination) in background
./qnetd 3 > qnetd3_output.log 2>&1 &
QNETD3_PID=$!
echo "[TEST] Started Node 3 (PID $QNETD3_PID)"

sleep 1 # Wait for startup

./nexus_shell <<EOF > test_output.log
admin
admin
sysmkdir docs
touch docs/note.txt content
cd docs
ls
cd ..
touch test_file.txt Hello_Quantum_World
ls
cat test_file.txt
ps
vision cat
vision dog
visualize qnn
net qkd 2 bb84
net qkd 3 bb84
net send 2 HELLO_WORLD
net send 2 MALICIOUS_CODE
net tomography 2
net repeater 3
net optimize 5
spawn traveler
spawn traveler
net optimize
qfork
qec_status
net qns ping 2
net qns status
net dqc run grover
touch bell.qsl "qalloc 2\nh 0\ncnot 0 1\nmeasure"
qrun bell.qsl
# Test QEdit
# Simulate input: add text, save, quit
printf "a\nHello from QEdit\n.\nw\nq\n" | qedit test_editor.txt
cat test_editor.txt
teleport 101 2
mount 2 /mnt/node2
cat /mnt/node2/secret.txt
cat audit.log
exit
EOF

# 3. Analyze Output
echo "[TEST] Analyzing Output..."

if grep -q "User logged in with key" test_output.log; then
    echo "[PASS] Audit Log (Login): Verified."
else
    echo "[FAIL] Audit Log Login Entry Missing."
fi

if grep -q "Created file 'test_file.txt'" test_output.log; then
    echo "[PASS] Audit Log (File Create): Verified."
else
    echo "[FAIL] Audit Log File Entry Missing."
fi

# Check for Blockchain Hash Format (Length > 64 chars in brackets)
if grep -q "\[[0-9a-f]\{64\}\]" test_output.log; then
    echo "[PASS] Audit Blockchain (Hash Chain): Verified."
else
    echo "[FAIL] Audit Blockchain Hash Missing."
fi

if grep -q "QEC Fidelity" test_output.log; then
    echo "[PASS] System Info & QEC: Verified."
else
    echo "[FAIL] System Info Missing."
fi

if grep -q "Inference Result: Quantum Cat" test_output.log; then
    echo "[PASS] Vision (Cat): Verified."
else
    echo "[FAIL] Vision (Cat) Failed."
fi

if grep -q "Inference Result: Schrodinger's Dog" test_output.log; then
    echo "[PASS] Vision (Dog): Verified."
else
    echo "[FAIL] Vision (Dog) Failed."
fi

if grep -q "Connected to Neural Predictor" test_output.log; then
    echo "[PASS] AI Scheduler: Verified."
else
    echo "[FAIL] AI Scheduler Failed."
fi
if grep -q "Hello_Quantum_World" test_output.log; then
    echo "[PASS] File System (LedgerFS): Verified."
else
    echo "[FAIL] File System Read/Write Failed."
fi

if grep -q "Quantum Neural Network Visualization" test_output.log; then
    echo "[PASS] QNN Visualization: Verified."
else
    echo "[FAIL] QNN Visualization Missing."
fi

if grep -q "Final Shared Secret" test_output.log; then
    echo "[PASS] Quantum Network (BB84): Verified."
else
    echo "[FAIL] QKD Protocol Failed."
fi

if grep -q "Multi-hop Routing: Verified" test_output.log; then
    echo "[PASS] Multi-hop Routing: Verified."
else
    echo "[FAIL] Multi-hop Routing Failed."
fi

if grep -q "Dropped Malicious Packet" test_output.log; then
    echo "[PASS] Neural Firewall: Verified."
else
    echo "[FAIL] Neural Firewall Failed."
fi

if grep -q "Reconstructed Density Matrix" test_output.log; then
    echo "[PASS] Quantum Tomography: Verified."
else
    echo "[FAIL] Tomography Failed."
fi

if grep -q "Swapping Complete" test_output.log; then
    echo "[PASS] Quantum Repeater: Verified."
else
    echo "[FAIL] Repeater Failed."
fi

if grep -q "Optimal Partition found" test_output.log; then
    echo "[PASS] QAOA Optimization: Verified."
else
    echo "[FAIL] QAOA Failed."
fi

if grep -q "Balancing Load using QAOA" test_output.log; then
    echo "[PASS] Scheduler Optimization: Verified."
else
    echo "[FAIL] Scheduler Optimization Failed."
fi

if grep -q "entered Superposition" test_output.log; then
    echo "[PASS] Quantum Fork: Verified."
else
    echo "[FAIL] Quantum Fork Failed."
fi

if grep -q "Corrected Bit-Flip" test_output.log; then
    echo "[PASS] Neural QEC: Verified."
else
    echo "[WARN] Neural QEC: No errors occurred (Random chance)."
fi

if grep -q "QNS-L4" test_output.log; then
    echo "[PASS] Quantum Network Stack: Verified."
else
    echo "[FAIL] QNS Failed."
fi

if grep -q "DQC" test_output.log; then
    echo "[PASS] Distributed Quantum Computing: Verified."
else
    echo "[FAIL] DQC Failed."
fi

if grep -q "File Unlocked" test_output.log; then
    echo "[PASS] Applied DQC (Real Impact): Verified."
else
    echo "[FAIL] Applied DQC Failed."
fi

if grep -q "QSL Execution Start" test_output.log; then
    echo "[PASS] Quantum Scripting Language: Verified."
else
    echo "[FAIL] QSL Failed."
fi

if grep -q "Hello from QEdit" test_output.log; then
    echo "[PASS] Quantum Text Editor (QEdit): Verified."
else
    echo "[FAIL] QEdit Failed."
fi

if grep -q "Teleportation Complete" test_output.log; then
    echo "[PASS] Process Teleportation: Verified."
else
    echo "[FAIL] Teleportation Failed."
fi

echo "=== Test Suite Completed ==="
cat test_output.log
rm test_output.log

# Cleanup
kill $QNETD2_PID
kill $QNETD3_PID
rm qnetd2_output.log
rm qnetd3_output.log
