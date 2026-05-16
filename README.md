# ![logo](https://raw.githubusercontent.com/DreamCoreRev/EonsCore/refs/heads/master/favicon.png) EonsCore (3.3.5)

[![GitHub Release](https://img.shields.io/github/release/DreamCoreRev/EonsCore.svg)](https://github.com/DreamCoreRev/EonsCore/releases) [![License: GPL 2.0](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](COPYING)

--------------


* [Build Status](#build-status)
* [Introduction](#introduction)
* [Requirements](#requirements)
* [Dependencies & Repacks](#dependencies--repacks)
* [Additional Resources](#additional-resources)
* [Install](#install)
* [Reporting issues](#reporting-issues)
* [Submitting fixes](#submitting-fixes)
* [Copyright](#copyright)
* [Authors &amp; Contributors](#authors--contributors)
* [Links](#links)



## Build Status

MSVC
:------------:
[![MSVC Build Status](https://github.com/DreamCoreRev/EonsCore/actions/workflows/windows-build-release.yml/badge.svg?branch=AzgathCore&event=push)](https://github.com/DreamCoreRev/EonsCore/actions/workflows/windows-build-release.yml)


## Introduction

EonsCore is a *MMORPG* Framework based on **TrinityCore 3.3.5a (Wrath of the Lich King)** with extensive custom modifications and additional content.

It is derived from *TrinityCore*, which itself comes from *MaNGOS*, the *Massive Network Game Object Server*, and is based on the code of that project with extensive changes over time to optimize, improve and cleanup the codebase at the same time as improving the in-game mechanics and functionality.

EonsCore builds upon this foundation with custom C++ modifications, advanced Lua/Eluna scripting systems, and specialized content designed for the **Eons** private server project.

It is completely open source; community involvement is highly encouraged.

If you wish to contribute ideas or code, please visit our site linked below or make pull requests to our [Github repository](https://github.com/DreamCoreRev/EonsCore/pulls).

For further information on the EonsCore project, please visit our project website at [Eons-World.eu](https://eons-world.eu).

## Requirements

Software requirements are available in the [wiki](https://trinitycore.info/en/install/requirements) for Windows, Linux and macOS.

**EonsCore-specific requirements:**
- CMake 3.27.8
- Boost 1.78.0
- OpenSSL 3.1.3
- MySQL 8.4.7
- VC++ Redistributable

## Dependencies & Repacks

All required dependencies and game data are available as pre-compiled repacks. Download from the official EonsCore releases at [github.com/DreamCoreRev/EonsCore/releases](https://github.com/DreamCoreRev/EonsCore/releases)

### Build Tools & Libraries
- **CMake** [3.27.8](https://github.com/DreamCoreRev/EonsCore/releases/tag/cmake-3.27.8) - Build configuration
- **Boost** [1.78.0](https://github.com/DreamCoreRev/EonsCore/releases/tag/boost_1_78_0) - C++ libraries
- **OpenSSL** [3.1.3](https://github.com/DreamCoreRev/EonsCore/releases/tag/Win64OpenSSL-3_1_3) - SSL/TLS encryption
- **MySQL** [8.4.7](https://github.com/DreamCoreRev/EonsCore/releases/tag/mysql-8.4.7) - Database server

### Game Data & Assets
- **Maps** [MapsEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/MapsEons) - World terrain data
- **VMaps** [VMapsEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/VMapsEons) - Vertical mesh data
- **MMaps** [MMapsEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/MMapsEons) - Movement pathfinding
- **DBC Files** [DBCEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/DBCEons) - Game constants & data
- **Cameras** [CamerasEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/CamerasEons) - Cinematic cameras
- **Lua** [5.2](https://github.com/DreamCoreRev/EonsCore/releases/tag/lua52) - Eluna scripting engine

### Compiled Binaries & Tools
- **Binaries** [BinEons](https://github.com/DreamCoreRev/EonsCore/releases/tag/BinEons) - Compiled servers & tools
- **Development Build** [build-db3d9258](https://github.com/DreamCoreRev/EonsCore/releases/tag/build-db3d9258) - Compiled servers & tools
- **VC++ Redistributable** [VC_redist](https://github.com/DreamCoreRev/EonsCore/releases/tag/VC_redist) - Runtime libraries
- **Git** [2.53.0](https://github.com/DreamCoreRev/EonsCore/releases/tag/Git-2.53.0.2-64-bit) - Version control
- **SQLyog** [13.3.1](https://github.com/DreamCoreRev/EonsCore/releases/tag/SQLyog-13.3.1-0.x64Community) - MySQL GUI
- **WowChat** [2.0.3](https://github.com/DreamCoreRev/EonsCore/releases/tag/wowchat-2.0.3) - Server console chat

## Additional Resources

Beyond the core server, EonsCore requires several essential components to operate:

### Client & Patching
- **Client Patch** [EonsPatch](https://github.com/DreamCoreRev/EonsPatch) - Essential client modifications for WoW 3.3.5 compatibility

### Web & Community
- **Website** [eons-world.eu](https://github.com/DreamCoreRev/eons-world.eu) - Official Eons website repository (PHP/MySQL CMS)
- **Launcher** [Eons-Launcher](https://github.com/DreamCoreRev/Eons-Launcher) - Game launcher application for easy server access

## Install

Detailed installation guides are available in the [wiki](https://trinitycore.info/en/home) for Windows, Linux and macOS.

**For EonsCore setup**, ensure you download all repacks listed above and follow the platform-specific installation documentation.

## Reporting issues

Issues can be reported via the [Github issue tracker](https://github.com/DreamCoreRev/EonsCore/issues).

Please take the time to review existing issues before submitting your own to prevent duplicates.

In addition, thoroughly read through the [issue tracker guide](https://community.trinitycore.org/topic/37-the-trinitycore-issuetracker-and-you/) to ensure your report contains the required information. Incorrect or poorly formed reports are wasteful and are subject to deletion.


## Submitting fixes

C++ fixes are submitted as pull requests via Github. For more information on how to properly submit a pull request, read the [how-to: maintain a remote fork](https://community.trinitycore.org/topic/9002-howto-maintain-a-remote-fork-for-pull-requests-tortoisegit/).

For SQL only fixes, open a ticket; if a bug report exists for the bug, post on an existing ticket.


## Copyright

License: GPL 2.0

Read file [COPYING](COPYING).


## Authors &amp; Contributors

Read file [AUTHORS](AUTHORS).

EonsCore builds upon the work of the TrinityCore team. See the [TrinityCore AUTHORS](https://github.com/TrinityCore/TrinityCore/blob/3.3.5/AUTHORS) file for their contributors.


## Links

* [Eons Website](https://eons-world.eu)
* [Eons Discord](https://discord.com/invite/KrQsUdUz8W)
* [Eons Repository](https://github.com/DreamCoreRev/EonsCore)
* [Eons Releases](https://github.com/DreamCoreRev/EonsCore/releases)
* [TrinityCore Wiki](https://trinitycore.info)
* [TrinityCore Forums](https://talk.trinitycore.org/)
* [TrinityCore Discord](https://discord.trinitycore.org/)
