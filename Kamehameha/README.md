# Jessi Lab - Kamehameha

Fabriquer un Kamehameha avec un Arduino Nano, un capteur ultrason et un ruban LED adressable !

## Matériel nécessaire

- Arduino Nano
- Capteur ultrason HC-SR04
- Ruban LED DotStar (30 LED)
- Câbles Dupont
- Câble USB
- Alimentation 5 V adaptée au ruban si nécessaire

## Branchements

| Composant | Arduino Nano |
|---|---|
| HC-SR04 TRIG | D4 |
| HC-SR04 ECHO | D5 |
| HC-SR04 VCC | 5V |
| HC-SR04 GND | GND |
| Ruban LED DATA | D6 |
| Ruban LED CLOCK | D7 |
| Ruban LED VCC | 5V |
| Ruban LED GND | GND |

## Logiciels nécessaires

- Arduino IDE
- Bibliothèque Adafruit DotStar
- Python 3
- PowerShell

## Installation

1. Ouvrir `kamehameha.ino` dans Arduino IDE.
2. Installer la bibliothèque Adafruit DotStar.
3. Sélectionner la carte Arduino Nano et son port COM.
4. Téléverser le programme.
5. Installer les dépendances Python dans PowerShell :

`pip install pygame pyserial`

6. Modifier `PORT = "COM4"` dans `kamehameha.py` selon le port de votre Arduino.
7. Fermer le moniteur série Arduino.
8. Dans PowerShell, se placer dans le dossier du programme Python et exécuter :

`python kamehameha.py`

## Fonctionnement

- Entre 10 et 60 cm : chargement du Kamehameha.
- À 10 cm ou moins : explosion.
- Au-delà de 15 cm : réarmement pour une nouvelle attaque.

## Jessi Lab

Projet électronique DIY réalisé par Jessi Lab.
