# discord-what-qua

> [!IMPORTANT]
> The tarball should be signed by github actions attestations. If you are not sure about the integrity of the tarball,
> please do not run the install script.
> The maintainer of this project is not responsible for any damage caused by the install script if the tarball is not
> signed by github actions attestations.

## Install

Prerequisites packages: - openssl - glibc
Determine which version of the tarball to download from the release page.
Tarballs with the suffix `-old-old` targets ubuntu 20.04 which need openssl v1.It does not support arm64.
Tarballs with the suffix `-old` targets ubuntu 22.04 which need openssl v3. It supports arm64.
Tarballs without the suffix targets ubuntu 24.04 which need openssl v3. It supports arm64.
After untar the tarball, you will need to change the token to your bot token.
Then make the install script executable and run it with sudo (Please make sure to check the content of the script before
running it with sudo).

> [!IMPORTANT]
> The tarball should be signed by github actions attestations. If you are not sure about the integrity of the tarball,
> please do not run the install script.
> The maintainer of this project is not responsible for any damage caused by the install script if the tarball is not
> signed by github actions attestations.

## Usage

### Stop the bot gracefully

Place a fine named `STOP` in the same directory as the bot. The bot will stop gracefully after calling the next command
in any of the servers.

### Stop the bot immediately

Control-C, it will not clean up the memery.
The cleanup will be done by the OS.

### LEGAL NOTICE

This project utilizes various Free and Open Source Software (FOSS) projects. For a comprehensive list of these projects
and their respective licenses, please refer to the NOTICE file located in the root directory of this repository.
