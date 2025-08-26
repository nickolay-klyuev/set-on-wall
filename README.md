# SetonWall

A small program that simplifies changing wallpapers on [sway-wm](https://swaywm.org/). 

## Usage

1. Without arguments, the program will search for all images in the current directory and allow the user to select an image to use as wallpaper interactively.
2. When passing the path to an image as the first argument, the program will attempt to set it as wallpaper.

After that you need to reload your sway-wm (swaymsg reload). Example: (setonwall && swaymsg reload)
