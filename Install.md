Prerequisites packages: - openssl - glibc
Determine which version of the tarball to download from the release page.
Tarball with `-old` has compatibility with glibc shipped with ubuntu 20.04 on amd64, and 22.04 on arm64.
Tarball without `-old` has compatibility with glibc shipped with ubuntu 24.04 on amd64, and 24.04 on arm64.
After untar the tarball, you will need to change the token.txt to your bot token.
Then make the install script executable and run it with sudo (Please make sure to check the content of the script before running it with sudo).

> [!IMPORTANT]
> The tarball should be signed by github actions attestations. If you are not sure about the integrity of the tarball, please do not run the install script.
> The maintainer of this project is not responsible for any damage caused by the install script if the tarball is not signed by github actions attestations.
