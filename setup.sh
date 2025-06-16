#!/bin/bash

# Create thirdparty directory
mkdir -p thirdparty
cd thirdparty

# Clone ImGui
if [ ! -d "imgui" ]; then
    echo "Cloning ImGui..."
    git clone https://github.com/ocornut/imgui.git
else
    echo "ImGui already exists, skipping..."
fi

# Clone Yoga
if [ ! -d "yoga" ]; then
    echo "Cloning Yoga layout engine..."
    git clone https://github.com/facebook/yoga.git
else
    echo "Yoga already exists, skipping..."
fi

# Clone TinyXML2
if [ ! -d "tinyxml2" ]; then
    echo "Cloning TinyXML2..."
    git clone https://github.com/leethomason/tinyxml2.git
else
    echo "TinyXML2 already exists, skipping..."
fi

echo "Setup complete!"
echo "Make sure you have SDL2 development libraries installed:"
echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev"
echo "  Fedora: sudo dnf install SDL2-devel"
echo "  Arch: sudo pacman -S sdl2"
echo ""
echo "To build the project:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"