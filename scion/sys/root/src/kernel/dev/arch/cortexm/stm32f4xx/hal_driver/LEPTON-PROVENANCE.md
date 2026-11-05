# Provenance (chantier environnement de développement Lepton, ENV-34)

`stm32f4xx-hal-driver` commit `b6f0ed3829f3829eb358a2e7417d80bba1a42db7` (tag `v1.8.5`),
sous-module de STM32CubeF4 v1.28.3 (https://github.com/STMicroelectronics/stm32f4xx-hal-driver),
BSD-3-Clause (`LICENSE.md`). `Inc/` et `Src/` complets (`Legacy/` et modèles `*_template.*`
compris), `LICENSE.md` et `README.md`, extraits par `git archive` et non modifiés (code tiers,
D1a) ; notes de version HTML, `_htmresc/`, `.github/` et fichiers de contribution non repris.
Configuration (`stm32f4xx_hal_conf.h`) fournie par le BSP de la carte.

Remplace `cubemx_hal_driver/` (HAL V1.7.1, sans provenance). Non compilée en ENV-34 ; le passage
de la SPL à la HAL fait l'objet d'ENV-35.

Contrôle : `scripts/tiers/verifier-import.sh <clone> v1.8.5 <ce répertoire> <liste>` (dépôt
lepton-tools), liste `Inc/`, `Src/`, `LICENSE.md`, `README.md` : 200 fichiers identiques.
Mise à jour : reprendre le commit du sous-module d'une version ultérieure de STM32CubeF4, avec le
CMSIS device du même couple (`kernel/core/ucore/cmsis-5/Device/ST/STM32F4xx/`).
