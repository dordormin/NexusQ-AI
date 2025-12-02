#!/bin/bash
nasm -f elf64 kernel/core/arch/x86/qpu_ctx_switch.asm -o kernel/core/arch/x86/qpu_ctx_switch.o

gcc -o nexus_kernel \
    kernel/core/arch/x86/qpu_ctx_switch.o \
    kernel/core/kmain.c \
    kernel/memory/kalloc.c \
    kernel/fs/ledgerfs.c \
    kernel/fs/qdfs.c \
    kernel/quantum/qec.c \
    kernel/net/qnet/qnet.c \
    kernel/net/qnet/purification.c \
    kernel/net/qnet/routing.c \
    kernel/net/qnet/qtcp.c \
    kernel/core/scheduler.c \
    kernel/core/qproc.c \
    kernel/core/svc_manager.c \
    kernel/core/klog.c \
    kernel/core/audit.c \
    kernel/core/syscalls.c \
    kernel/core/migration.c \
    kernel/core/stubs.c \
    kernel/neural/nal.c \
    kernel/neural/backends.c \
    modules/crypto/ledgerfs/hash.c \
    modules/crypto/wallet/wallet.c \
    modules/quantum/qaoa.c \
    modules/quantum/teleport.c \
    modules/connectivity/bt_asm.s \
    modules/connectivity/bluetooth.c \
    modules/quantum/dqc.c \
    modules/quantum/qns_layer.c \
    modules/quantum/tomography.c \
    modules/quantum/repeater.c \
    modules/quantum/visualization.c \
    modules/quantum/qec_neural.c \
    modules/quantum/qvm.c \
    modules/quantum/qdbg.c \
    modules/quantum/qmonitor.c \
    modules/quantum/qopt.c \
    modules/quantum/qexport.c \
    modules/quantum/qvis.c \
    modules/quantum/qprof.c \
    modules/quantum/noise.c \
    modules/quantum/qec_sim.c \
    modules/quantum/qkd.c \
    modules/neural/qnn_xor.c \
    modules/quantum/qhal.c \
    modules/quantum/mapper.c \
    kernel/core/governance.c \
    modules/graphics/gpu.c \
    modules/ui/compositor.c \
    modules/contracts/vm/vm.c \
    -I include \
    -I kernel/memory/include \
    -I kernel/neural/include \
    -I kernel/core/include \
    -I modules/crypto/include \
    -I modules/graphics/include \
    -I modules/contracts/include \
    -I modules/quantum/include \
    -lm

echo "[BUILD] Compiling Nexus Shell..."
gcc -o nexus_shell \
    apps/shell.c \
    apps/qedit.c \
    apps/qsl.c \
    lib/libnexus.c \
    kernel/core/arch/x86/qpu_ctx_switch.o \
    kernel/memory/kalloc.c \
    kernel/fs/ledgerfs.c \
    kernel/fs/qdfs.c \
    kernel/quantum/qec.c \
    kernel/net/qnet/qnet.c \
    kernel/net/qnet/purification.c \
    kernel/net/qnet/routing.c \
    kernel/net/qnet/qtcp.c \
    kernel/core/scheduler.c \
    kernel/core/qproc.c \
    kernel/core/svc_manager.c \
    kernel/core/klog.c \
    kernel/core/audit.c \
    kernel/core/syscalls.c \
    kernel/core/migration.c \
    kernel/core/stubs.c \
    kernel/neural/nal.c \
    kernel/neural/backends.c \
    modules/crypto/ledgerfs/hash.c \
    modules/crypto/wallet/wallet.c \
    modules/quantum/qaoa.c \
    modules/quantum/teleport.c \
    modules/connectivity/bt_asm.s \
    modules/connectivity/bluetooth.c \
    modules/quantum/dqc.c \
    modules/quantum/qns_layer.c \
    modules/quantum/tomography.c \
    modules/quantum/repeater.c \
    modules/quantum/visualization.c \
    modules/quantum/qec_neural.c \
    modules/quantum/qvm.c \
    modules/quantum/qdbg.c \
    modules/quantum/qmonitor.c \
    modules/quantum/qopt.c \
    modules/quantum/qexport.c \
    modules/quantum/qvis.c \
    modules/quantum/qprof.c \
    modules/quantum/noise.c \
    modules/quantum/qec_sim.c \
    modules/quantum/qkd.c \
    modules/neural/qnn_xor.c \
    modules/quantum/qhal.c \
    modules/quantum/mapper.c \
    kernel/core/governance.c \
    modules/graphics/gpu.c \
    modules/ui/compositor.c \
    modules/contracts/vm/vm.c \
    -I include \
    -I kernel/memory/include \
    -I kernel/neural/include \
    -I kernel/core/include \
    -I modules/crypto/include \
    -I modules/graphics/include \
    -I modules/contracts/include \
    -I modules/quantum/include \
    -lm -ldl

