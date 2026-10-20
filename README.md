# pickme - a minimal applauncher

## status

![Status](https://img.shields.io/badge/Status-In%20Testing%20Phase-yellow)

## Why this is great

1. pure C: only libawayland + cairo
2. Wayland only

### short Ai-disclaimer

Currently the UI is fully done with ai, because currently my knowledge of keyboard inputs and wayland
is not enough, aim is to later write it by myself :).
The rest is fully written by me, every line.

## how to install

- clone repository
- cd into repo
- install dependencies, have a wayland environment:

```sh
echo $XDG_SESSION_TYPE #should return wayland
sudo pacman -S cairo
```

- make release

## how to use

1. command line/logic modus:

```sh
./pickme --logic
```

1.

```sh
./pickme
```

## features

- you can manually add applications via ~/.config/pickme/my_application.desktop,
  if something is not captured by auto detecting
- let the window float in hyprland by adding something like this to the lua config:

```lua
hl.window_rule({
 name = "pickme-float",
 match = { class = "^(pickme)$" },
 float = true,
 center = true,
 size = "600 400",
})
```

### NOTE

I currently have only tested it on my system(artix-linux - hyprland),
there could be issues in other wayland de's like gnome, kde, cosmic ...,
feel free to tell me if it works on your setup

## contributing

It would be great if you help me add some test's, there are currently to few
I am using ceedling with unity, you could just edit ./test/test\_\*.c
create a pull request.
You could test your work with:

```sh
ceedling test:fuzzy #if you addded something to fuzzy module
make test # or just type ceedling to test all
```
