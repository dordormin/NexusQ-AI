# Guide de Test - Résolutions des Problèmes QEdit et QVM

## Problèmes Résolus

### 1. ✅ Les fichiers ne se dupliquent plus dans tous les dossiers
- **Avant**: Un fichier créé dans /quantum apparaissait aussi dans /docs
- **Maintenant**: Les fichiers sont isolés par `parent_id` (répertoire parent)

### 2. ✅ Les copies "(1)" n'apparaissent que dans le même dossier
- **Avant**: Duplication globale avec numéros de copie partout
- **Maintenant**: Vérification du nom uniquement dans le même `parent_id`

### 3. ✅ QEdit sauvegarde correctement les fichiers
- **Avant**: Utilisait `lfs_create_file` directement avec parent_id=0 (root)
- **Maintenant**: Utilise `nexus_create_file` avec le `cwd_id` correct

### 4. ✅ La commande `qexec` fonctionne
- **Avant**: "Unknown command: qexec"
- **Maintenant**: Intégrée dans le main loop du shell

## Tests à Effectuer

### Test 1: Isolation des Dossiers
```bash
mkdir quantum
cd quantum
qedit test1.qc
# Tapez: a
# Puis: QUBITS 2
# Puis: H 0
# Puis: .
# Puis: wq

cd /
mkdir docs  
cd docs
ls          # test1.qc NE devrait PAS apparaître ici!
```

### Test 2: QEdit Sauvegarde dans le Bon Dossier
```bash
cd /quantum
qedit bell.qc
# Dans qedit:
:a
  > QUBITS 2
  > H 0
  > CNOT 0 1
  > M 0
  > M 1
  > .
:wq

ls          # bell.qc devrait être visible
cat bell.qc # Devrait afficher le contenu
```

### Test 3: Exécution QVM
```bash
qexec bell.qc
# Devrait afficher:
# [QVM] Initialized 2-qubit state
# [QVM] Executing circuit with X gates...
# [QVM] Measured qubit 0: |0⟩ ou |1⟩
# [QVM] Measured qubit 1: |0⟩ ou |1⟩
# --- Quantum State ---
# ...probabilités...
```

### Test 4: Circuits de Test Fournis
```bash
cd /
qexec test_bell.qc          # Test d'intrication
qexec test_superposition.qc # Test de superposition
qexec test_grover.qc        # Algorithme de Grover simplifié
```

## Commandes QEdit Disponibles
- `a` ou `add` - Ajouter des lignes (terminer avec `.`)
- `p` ou `list` - Afficher toutes les lignes
- `d <num>` - Supprimer la ligne numéro <num>
- `w` ou `save` - Sauvegarder
- `q` ou `quit` - Quitter
- `wq` ou `x` - Sauvegarder ET quitter 🎯
- `q!` - Forcer la sortie sans sauvegarder
- `h` ou `help` - Afficher l'aide

## Résumé Technique des Changements

1. **ledgerfs.c**: `lfs_create_file()` vérifie maintenant `parent_id` en plus du nom
2. **qedit.c**: Utilise `nexus_create_file(name, content, cwd_id)` au lieu de `lfs_create_file(name, content, 0, ...)`
3. **shell.c**: Ajout du handler `qexec` dans le main loop

Tous les problèmes sont maintenant corrigés ! 🚀
