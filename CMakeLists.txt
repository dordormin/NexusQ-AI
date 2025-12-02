cmake_minimum_required(VERSION 3.20)
project(NexusQ_AI)

set(CMAKE_CXX_STANDARD 14)
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -f elf64")

include_directories(kernel/memory/include)
include_directories(kernel/memory/include/sys)

add_executable(NexusQ_AI
        boot/boot.asm
        kernel/core/qproc.c
        kernel/core/scheduler.c

        kernel/fs/ledgerfs.c
        kernel/net/qnet/qnet.c
        kernel/quantum/qec.c
        kernel/memory/include/sys/kalloc.h
        kernel/memory/include/sys/ledgerfs.h
        kernel/memory/include/sys/qproc.h
        kernel/memory/include/sys/scheduler.h
        kernel/memory/kalloc.c
        tests/test_integration_1.c
        tests/test_kalloc_unit.c
        tests/test_ledgerfs.c
        tests/test_scheduler.c)

# Executable pour les tests de Gouvernance
add_executable(test_governance
        tests/test_governance.c
        kernel/core/syscalls.c
        kernel/fs/ledgerfs.c
        kernel/fs/ledgerfs.c
        kernel/net/qnet/qnet.c
        kernel/quantum/qec.c
        kernel/core/qproc.c
        kernel/core/scheduler.c
        kernel/memory/kalloc.c
        modules/neural/inference/predict.c
        modules/neural/training/train.c
        modules/crypto/ledgerfs/hash.c
        modules/crypto/wallet/wallet.c
        modules/contracts/vm/vm.c
        modules/graphics/gpu.c
        modules/quantum/visualization.c
        modules/neural/graphics/nss.c
        modules/ui/compositor.c)

# Demo Application (User Space)
add_executable(app_demo
        kernel/core/audit.c
        apps/app_demo.c
        lib/libnexus.c
        kernel/core/syscalls.c
        kernel/fs/ledgerfs.c
        kernel/fs/qdfs.c
        kernel/net/qnet/qnet.c
        kernel/net/qnet/routing.c
        kernel/net/qnet/qtcp.c
        kernel/quantum/qec.c
        kernel/core/arch/x86/qpu_ctx_switch.asm
        modules/neural/arch/x86/neural_ops.asm
        modules/neural/qnn/pqc.c
        modules/crypto/arch/x86/sha256_core.asm
        kernel/core/qproc.c
        kernel/core/migration.c
        kernel/core/scheduler.c
        kernel/memory/kalloc.c
        modules/neural/inference/predict.c
        modules/neural/training/train.c
        modules/crypto/ledgerfs/hash.c
        modules/crypto/wallet/wallet.c
        modules/contracts/vm/vm.c
        modules/graphics/gpu.c
        modules/quantum/visualization.c
        modules/neural/graphics/nss.c
        modules/ui/compositor.c
        kernel/neural/nal.c
        modules/vision/vision_core.c
        modules/neural/models/scheduler_model.c
        modules/neural/models/firewall_model.c
        modules/quantum/tomography.c
        modules/quantum/repeater.c
        modules/quantum/qaoa.c
        modules/quantum/qec_neural.c
        modules/quantum/qns_layer.c
        modules/quantum/dqc.c)

# Nexus Shell (Interactive CLI)
add_executable(nexus_shell
        kernel/core/audit.c
        apps/shell.c
        apps/qsl.c
        apps/qedit.c
        lib/libnexus.c
        kernel/core/syscalls.c
        kernel/fs/ledgerfs.c
        kernel/fs/qdfs.c
        kernel/net/qnet/qnet.c
        kernel/net/qnet/routing.c
        kernel/net/qnet/qtcp.c
        kernel/quantum/qec.c
        kernel/core/arch/x86/qpu_ctx_switch.asm
        modules/neural/arch/x86/neural_ops.asm
        modules/neural/qnn/pqc.c
        modules/crypto/arch/x86/sha256_core.asm
        kernel/core/qproc.c
        kernel/core/migration.c
        kernel/core/scheduler.c
        kernel/memory/kalloc.c
        modules/neural/inference/predict.c
        modules/neural/training/train.c
        modules/crypto/ledgerfs/hash.c
        modules/crypto/wallet/wallet.c
        modules/contracts/vm/vm.c
        modules/graphics/gpu.c
        modules/quantum/visualization.c
        modules/neural/graphics/nss.c
        modules/ui/compositor.c
        kernel/neural/nal.c
        modules/vision/vision_core.c
        modules/neural/models/scheduler_model.c
        modules/neural/models/firewall_model.c
        modules/quantum/tomography.c
        modules/quantum/repeater.c
        modules/quantum/qaoa.c
        modules/quantum/qec_neural.c
        modules/quantum/qns_layer.c
        modules/quantum/dqc.c)

target_link_libraries(nexus_shell m)
target_link_libraries(app_demo m)
target_link_libraries(test_governance m)

# Test QEC
add_executable(test_qec
        tests/test_qec.c
        kernel/quantum/qec.c
        kernel/core/qproc.c
        kernel/memory/kalloc.c)
target_link_libraries(test_qec m)

# Service: QNet Daemon
add_executable(qnetd
        services/qnetd.c
        lib/libnexus.c
        kernel/core/syscalls.c
        kernel/fs/ledgerfs.c
        kernel/fs/qdfs.c
        kernel/net/qnet/qnet.c
        kernel/net/qnet/routing.c
        kernel/net/qnet/qtcp.c
        kernel/quantum/qec.c
        kernel/core/arch/x86/qpu_ctx_switch.asm
        modules/neural/arch/x86/neural_ops.asm
        modules/neural/qnn/pqc.c
        modules/crypto/arch/x86/sha256_core.asm
        kernel/core/qproc.c
        kernel/core/migration.c
        kernel/core/scheduler.c
        kernel/memory/kalloc.c
        modules/neural/inference/predict.c
        modules/neural/training/train.c
        modules/crypto/ledgerfs/hash.c
        modules/crypto/wallet/wallet.c
        modules/contracts/vm/vm.c
        modules/graphics/gpu.c
        modules/quantum/visualization.c
        modules/neural/graphics/nss.c
        modules/ui/compositor.c
        kernel/core/audit.c
        kernel/neural/nal.c
        modules/vision/vision_core.c
        modules/neural/models/scheduler_model.c
        modules/neural/models/firewall_model.c
        modules/quantum/tomography.c
        modules/quantum/repeater.c
        modules/quantum/qaoa.c
        modules/quantum/qec_neural.c
        modules/quantum/qns_layer.c
        modules/quantum/dqc.c)
target_link_libraries(qnetd m)
