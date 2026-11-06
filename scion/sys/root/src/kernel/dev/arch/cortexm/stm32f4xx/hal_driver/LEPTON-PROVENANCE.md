# Provenance (chantier environnement de développement Lepton, ENV-34, ENV-41)

`stm32f4xx-hal-driver` tag `v1.8.5` = commit `b6f0ed3829f3829eb358a2e7417d80bba1a42db7`,
sous-module de STM32CubeF4 v1.28.3 (amont https://github.com/STMicroelectronics/stm32f4xx-hal-driver),
BSD-3-Clause (`LICENSE.md`). Code tiers non modifié (D1a).

Depuis ENV-41, ce répertoire n'est plus une copie : scion **projette** les fichiers du dépôt tiers
(clé `tiers@stm32f4xx-hal-driver`, déclarée dans `scion/.scion/` de ce greffon, qui la porte).
Source : miroir de conservation https://github.com/lepton-distribution/mirror-stm32f4xx-hal-driver.git
(tenu par `scion mirror-sync`, `scion/.scion/.scion.mirrors`) ; clone partiel sous
`<rootstock>/depots/tiers/stm32f4xx-hal-driver/v1.8.5`, au commit du tag. Fichiers projetés :
`Inc/**`, `Src/**` (`Legacy/` et modèles `*_template.*` compris), `LICENSE.md`, `README.md`
(200 fichiers) ; notes de version HTML, `_htmresc/`, `.github/` et fichiers de contribution non
projetés. Seul ce fichier appartient au greffon. Configuration (`stm32f4xx_hal_conf.h`) fournie par
le BSP de la carte (`scripts/tiers/hal_conf_stm32f4.py` de lepton-tools).

Contenu : identique à `v1.8.5` par construction (liens vers le clone tiers, que la greffe refuse
hors de son commit épinglé ou modifié). Exige scion ≥ 0.6.1.0.
Mise à jour : changer la version de la clé dans `scion/.scion/` (`.scion.sources.list`,
`.scion.ramifications`), avec le CMSIS device du même couple STM32CubeF4
(`kernel/core/ucore/cmsis-5/Device/ST/STM32F4xx/`), puis regreffer.
