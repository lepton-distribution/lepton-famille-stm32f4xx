# Fichiers SVD de la famille stm32f4xx

Vue des périphériques au débogage (DBG-4). Le BSP désigne son fichier par
`"svd": {"famille": "svd/<fichier>"}` dans son `lepton-greffon.json`.

Fichiers copiés sans modification du pack CMSIS `Keil::STM32F4xx_DFP` 3.1.1 (2025-08-18), téléchargé de
https://www.keil.com/pack/Keil.STM32F4xx_DFP.3.1.1.pack le 2026-11-02 (ENV-26, décision PA-39).

| Fichier | MCU | Chemin dans le pack | Octets | SHA-256 |
| --- | --- | --- | --- | --- |
| `STM32F429.svd` | STM32F429ZI (BSP nucleo-f429zi) | `CMSIS/SVD/STM32F429.svd` | 2118587 | `185ddfe09ebec697628850d3c262092c477924ff618d29cf3a2d6e453b44336b` |
| `STM32F439.svd` | STM32F439ZI (BSP nucleo-f439zi) | `CMSIS/SVD/STM32F439.svd` | 2071506 | `c3de4e42c9fa2e0196f364efa25d163e4a9813b6c6b2fd0f5a8ffb64158851c9` |

Licence : Apache-2.0, selon l'en-tête de chaque fichier (`SPDX-License-Identifier: Apache-2.0`),
conservé tel quel. Texte de la licence : `LICENSE-Apache-2.0`.
