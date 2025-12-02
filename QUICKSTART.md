# 🚀 Guide de Démarrage Rapide - QVM NexusQ-AI

## Installation & Compilation

```bash
# 1. Compiler le système
./build_kernel.sh

# 2. Compiler les tests (optionnel)
./build_tests.sh

# 3. Lancer le shell
./nexus_shell
```

## 🎯 Premiers Pas (5 minutes)

### 1. Votre Premier Circuit Quantique

```bash
# Dans le shell NexusQ :
nexus@...> qedit hello.qc

# Dans qedit :
:a
  > QUBITS 1
  > H 0
  > MEASURE 0
  > .
:wq

# Exécuter le circuit
nexus@...> qexec hello.qc
```

**Résultat** : Vous créez une superposition quantique et mesurez le résultat !

---

### 2. Explorer la Bibliothèque

```bash
# Téléportation quantique (wow factor!)
nexus@...> qexec quantum_lib/teleportation.qc

# État GHZ (intrication 3 qubits)
nexus@...> qexec quantum_lib/ghz_state.qc

# Générateur de nombres aléatoires quantiques
nexus@...> qexec quantum_lib/qrng.qc
```

---

### 3. Debug Interactif

```bash
nexus@...> qdbg quantum_lib/ghz_state.qc

# Dans le debugger :
qdbg> n          # Next (exécuter prochaine porte)
qdbg> s          # State (voir l'état quantique)
qdbg> g          # Gates (lister toutes les portes)
qdbg> c          # Continue (exécuter jusqu'à la fin)
qdbg> q          # Quit
```

---

## 📚 Circuits Disponibles

| Fichier | Description | Niveau |
|---------|-------------|--------|
| `test_bell.qc` | État de Bell (intrication 2 qubits) | Débutant |
| `test_superposition.qc` | Superposition 3 qubits | Débutant |
| `quantum_lib/ghz_state.qc` | État GHZ | Intermédiaire |
| `quantum_lib/teleportation.qc` | Téléportation quantique | Avancé |
| `quantum_lib/deutsch.qc` | Algorithme de Deutsch | Intermédiaire |
| `quantum_lib/qrng.qc` | Générateur aléatoire | Débutant |

---

## 🔧 Commandes Utiles

```bash
# Système de fichiers
ls                          # Lister les fichiers
cd quantum_lib              # Changer de dossier
mkdir mon_projet            # Créer un dossier
pwd                         # Dossier actuel

# Édition de circuits
qedit <fichier>             # Éditer/créer un circuit
cat <fichier>               # Voir le contenu

# Exécution quantique
qexec <fichier>             # Exécuter un circuit
qdbg <fichier>              # Debugger un circuit

# Tests
# (Dans bash, pas dans nexus_shell)
./test_qvm                  # Lancer les tests unitaires
```

---

## 🎓 Tutoriel: Créer un Circuit depuis Zéro

### Étape 1: Créer le fichier
```bash
nexus@SYSTEM_R:/> mkdir mes_circuits
nexus@SYSTEM_R:/> cd mes_circuits
nexus@SYSTEM_R:/mes_circuits/> qedit mon_premier.qc
```

### Étape 2: Écrire le circuit
```
:a
  > # Mon premier circuit quantique
  > QUBITS 2
  > 
  > # Créer une superposition sur qubit 0
  > H 0
  > 
  > # Intriquer avec qubit 1
  > CNOT 0 1
  > 
  > # Mesurer les deux
  > MEASURE 0
  > MEASURE 1
  > .
:wq
```

### Étape 3: Exécuter et debugger
```bash
# Exécution normale
nexus@SYSTEM_R:/mes_circuits/> qexec mon_premier.qc

# Debug pas à pas
nexus@SYSTEM_R:/mes_circuits/> qdbg mon_premier.qc
qdbg> n    # Exécuter H 0
qdbg> s    # Voir l'état (superposition)
qdbg> n    # Exécuter CNOT
qdbg> s    # Voir l'intrication!
qdbg> c    # Finir l'exécution
qdbg> q    # Quitter
```

---

## 📖 Format de Circuit (.qc)

```
# Commentaires commencent par #

# Déclarer le nombre de qubits (obligatoire)
QUBITS <nombre>

# Portes quantiques disponibles:
H <qubit>           # Hadamard (superposition)
X <qubit>           # Pauli-X (NOT)
Y <qubit>           # Pauli-Y
Z <qubit>           # Pauli-Z (phase)
T <qubit>           # T gate (π/8)
S <qubit>           # S gate (π/4)
CNOT <ctrl> <tgt>   # Controlled-NOT
MEASURE <qubit>     # Mesure
M <qubit>           # Mesure (raccourci)
```

---

## 🐛 Dépannage

**Problème**: `qexec: Unknown command`  
**Solution**: Compiler avec `./build_kernel.sh` puis relancer `./nexus_shell`

**Problème**: Fichier non trouvé  
**Solution**: Vérifier avec `ls` et `pwd`. Utiliser le chemin complet si nécessaire.

**Problème**: État quantique bizarre  
**Solution**: Utiliser `qdbg` pour voir étape par étape ce qui se passe.

---

## 🎯 Challenges

1. **Débutant**: Créer un circuit qui génère un nombre aléatoire entre 0 et 7
2. **Intermédiaire**: Créer un circuit qui produit l'état |+⟩ = (|0⟩ + |1⟩)/√2
3. **Avancé**: Implémenter l'algorithme de Grover pour 3 qubits

---

## 📊 Vérifier que Tout Fonctionne

```bash
# 1. Compiler
./build_kernel.sh
./build_tests.sh

# 2. Tests unitaires
./test_qvm
# Devrait afficher: 🎉 All tests passed!

# 3. Test d'intégration
./nexus_shell
# Dans le shell:
nexus@SYSTEM_R:/> qexec quantum_lib/qrng.qc
# Devrait générer un nombre aléatoire

# 4. Test du debugger
nexus@SYSTEM_R:/> qdbg test_bell.qc
qdbg> n
qdbg> s
qdbg> q
```

Si tout fonctionne : **Félicitations !** 🎉  
Votre système quantique est opérationnel !

---

**Prochaines étapes** : Consultez `QVM_IMPLEMENTATIONS.md` pour plus de détails
