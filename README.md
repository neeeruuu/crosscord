<h1 align="center">
    <img src="resources/icon.png" alt="icon" width="128" height="128">
    <br/>
    <b>CrossCord</b>
</h1>

<p align="center">
    An anticheat compatible crosshair that uses Discord's overlay
    <br/>
    <a href="https://github.com/neeeruuu/crosscord/releases/latest"><b>Download</b></a>
    <br/>
    <br/>
    Inspired by <a href="https://github.com/SamuelTulach">SamuelTulach</a>'s work
</p>


# Usage
1. Enable Discord's overlay
2. Open CrossCord
3. The moment Discord starts drawing an overlay on a game, CrossCord will draw an overlay on it

# To-do
- [x] Basic crosshair types (cross, triangle, circle)
- [x] UI
- [x] Tray icon
- [x] Persistent settings
- [x] Icon
- [ ] Improve render stability
- [ ] Outlines
- [ ] GPU rendering
- [ ] Image crosshairs
- [ ] Framebuffer write callbacks instead of loop thread (possibly using PAGE_GUARD)
- [ ] Keybinds
- [ ] Improve comments

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
