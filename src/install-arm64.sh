#! /bin/bash

sudo mv what-bot-arm64.service ~/.config/systemd/user/
sudo chmod 664 ~/.config/systemd/user/what-bot-arm64.service
mkdir -p ~/.local/bin
mv bot-linux-arm64 ~/.local/bin/what-bot-arm64
sudo chmod +x ~/.local/bin/what-bot-arm64
sudo systemctl daemon-reload
systemctl --user enable what-bot-arm64
systemctl --user start what-bot-arm64