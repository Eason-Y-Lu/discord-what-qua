#! /bin/bash

mkdor -p ~/.config/systemd/user
mv what-bot-x64.service ~/.config/systemd/user/
sudo chmod 664 ~/.config/systemd/user/what-bot-x64.service
mkdir -p ~/.local/bin
mv bot-linux-x64 ~/.local/bin/what-bot-x64
sudo chmod +x ~/.local/bin/what-bot-x64
sudo systemctl daemon-reload
systemctl --user enable what-bot-x64
systemctl --user start what-bot-x64