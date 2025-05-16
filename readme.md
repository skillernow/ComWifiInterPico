# Communication Wi-Fi entre Raspberry Pi Pico W (AP + Client TCP)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Projet de communication Wi-Fi avancée entre Raspberry Pi Pico W utilisant un point d'accès (AP) et un client TCP en langage C.

## 📋 Description

Ce projet démontre une communication bidirectionnelle entre deux Raspberry Pi Pico W :
1. **Point d'accès (AP) avec serveur TCP** : Crée un réseau Wi-Fi et écoute les connexions entrantes
2. **Client Wi-Fi avec client TCP** : Se connecte au réseau et communique avec le serveur

Fonctionnalités principales :
- Configuration flexible du réseau Wi-Fi
- Échange de données TCP/IP
- Architecture client-serveur légère
- Optimisé pour les microcontrôleurs

## 🛠 Prérequis

### Matériel
- 2 × Raspberry Pi Pico W
- Câbles USB
- Ordinateur de développement

### Logiciel
- [Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk)
- CMake (version 3.12+)
- Compilateur arm-none-eabi-gcc
- IDE (VS Code, CLion, etc.)

## 🚀 Installation

1. Cloner le dépôt :
```bash
git clone https://github.com/skillernow/ComWifiInterPico.git
cd ComWifiInterPico

.
├── CMakeLists.txt
├── include/
│   ├── config.h
│   ├── wifi_ap.h
│   └── wifi_client.h
├── src/
│   ├── main_ap.c
│   ├── main_client.c
│   ├── wifi_ap.c
│   └── wifi_client.c
└── README.md

🔄 Workflow de communication
L'AP démarre et crée le réseau Wi-Fi

Le serveur TCP écoute sur le port spécifié

Le client se connecte au réseau Wi-Fi

Établissement de la connexion TCP

Échange bidirectionnel de données