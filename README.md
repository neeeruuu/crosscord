<h3 align="center">CrossCord</h3>

<p align="center">
    An anticheat compatible crosshair that uses Discord's overlay
    <br/>
    <a href="https://github.com/neeeruuu/crosscord/releases/latest"><strong>Download</strong></a>
    <br/>
    <br/>
    Inspired by <a href="https://github.com/SamuelTulach">SamuelTulach</a>'s work
</p>


# Usage
1. Enable Discord's overlay
2. Open CrossCord
3. The moment Discord starts drawing an overlay on a game, CrossCord will draw an overlay on it

# To-do
- [x] Basic crosshair types (cross, triangle)
- [x] Settings UI
- [x] Tray icon
- [ ] Use PAGE_GUARD or something else for detecting framebuffer changes
- [ ] Sphere crosshair
- [ ] Image crosshair
- [ ] Icon

# Building
### Requirements
* CMake
* Visual Studio

### Steps
1. Clone the repo ````
git clone https://github.com/neeeruuu/crosscord --recurse-submodules````
2. Open repo's folder
3. Run GenProj.bat (or open on VS Code and press Ctrl+Shift+B)
4. Open solution proj folder (crosscord.sln)
5. Build
