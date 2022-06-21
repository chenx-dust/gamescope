#!/bin/bash

sudo pacman -Sy gcc-libs glibc libcap.so=2-64 libdrm libliftoff.so=0-64 libpipewire-0.3.so=0-64 libvulkan.so=1-64 libwlroots.so=10-64 libx11 libxcomposite libxdamage libxext libxkbcommon.so=0-64 libxrender libxres libxtst libxxf86vm sdl2 vulkan-icd-loader wayland wayland-protocols wlroots xorg-server-xwayland git glslang meson ninja vulkan-headers pixman linux-api-headers mesa udev seatd libinput libxfixes libxcb xcb-util-wm libxi xcb-util-errors libglvnd xorgproto
git clone -b oxp-90-intel https://github.com/ruineka/gamescope-onexplayer
cd gamescope-onexplayer
meson build/
ninja -C build/
sudo cp ~/gamescope-onexplayer/build/gamescope /usr/bin

git clone -b service-test https://github.com/shadowblip/handygccs
cd handygccs
sudo make
reboot
