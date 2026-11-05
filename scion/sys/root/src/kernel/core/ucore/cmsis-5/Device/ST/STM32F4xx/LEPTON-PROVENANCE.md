# Provenance (chantier environnement de développement Lepton, ENV-34)

`cmsis_device_f4` commit `3c77349ce04c8af401454cc51f85ea9a50e34fc1` (sans tag ; `git describe` :
`v2.6.11-1-g3c77349`, V2.6.11 et un commit), sous-module de STM32CubeF4 v1.28.3
(https://github.com/STMicroelectronics/cmsis_device_f4), Apache-2.0 (`LICENSE.md`). Fichiers
extraits par `git archive` et non modifiés (code tiers, D1a) ; sous-ensemble :
`Include/stm32f4xx.h`, `Include/system_stm32f4xx.h`, `Include/stm32f429xx.h` (NUCLEO-F429ZI),
`Include/stm32f439xx.h` (NUCLEO-F439ZI), `LICENSE.md`, `README.md` ; les autres puces et les
modèles `Source/Templates` (démarrage, éditions de liens IAR/ARM/GCC) ne sont pas repris.
Ajouter l'en-tête d'une autre puce depuis le même commit si une carte l'exige.

Non compilé en ENV-34 : la SPL compilée garde ses propres en-têtes (`driverlib/`,
`kernel/core/ucore/cmsis/Device/st/stm32f4xx/`).

Contrôle : `scripts/tiers/verifier-import.sh <clone> 3c77349 <ce répertoire> <liste>` (dépôt
lepton-tools), liste des 6 fichiers ci-dessus : 6 fichiers identiques.
