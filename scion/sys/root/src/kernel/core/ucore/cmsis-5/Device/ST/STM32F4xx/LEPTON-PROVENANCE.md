# Provenance (chantier environnement de développement Lepton, ENV-34, ENV-41)

`cmsis_device_f4` commit `3c77349ce04c8af401454cc51f85ea9a50e34fc1` (sans tag ; `git describe` :
`v2.6.11-1-g3c77349`, V2.6.11 et un commit), sous-module de STM32CubeF4 v1.28.3
(amont https://github.com/STMicroelectronics/cmsis_device_f4), Apache-2.0 (`LICENSE.md`). Code
tiers non modifié (D1a).

Depuis ENV-41, ce répertoire n'est plus une copie : scion **projette** les fichiers du dépôt tiers
(clé `tiers@cmsis-device-f4`, déclarée dans `scion/.scion/` du greffon `family-stm32f4xx`, qui la
porte). Source : miroir de conservation https://github.com/lepton-distribution/mirror-cmsis_device_f4.git
(tenu par `scion mirror-sync`) ; clone partiel sous
`<rootstock>/depots/tiers/cmsis-device-f4/3c77349ce04c8af401454cc51f85ea9a50e34fc1`. Fichiers
projetés : `Include/stm32f4xx.h`, `Include/system_stm32f4xx.h`, `Include/stm32f429xx.h`
(NUCLEO-F429ZI), `Include/stm32f439xx.h` (NUCLEO-F439ZI), `LICENSE.md`, `README.md` ; les autres
puces et les modèles `Source/Templates` ne sont pas projetés. Pour une autre puce : ajouter son
en-tête au motif de `scion/.scion/.scion.projections`. Seul ce fichier appartient au greffon.

Contenu : identique au commit 3c77349 par construction (liens vers le clone tiers, que la greffe
refuse hors de son commit épinglé ou modifié). Exige scion ≥ 0.6.1.0.
