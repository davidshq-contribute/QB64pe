# Linux Display Troubleshooting: Icon But No Window

If QB64-PE shows an app icon (in the taskbar or dock) but no visible window, the process is running and the window is being created, but something is preventing it from displaying correctly. This is often related to **Wayland**, **XWayland**, or **virtualization** (e.g. Gnome Boxes).

## Why this happens

- **QB64-PE’s GUI uses FreeGLUT**, which on Linux is **X11-only** (it uses Xlib/XCreateWindow and GLX). It does not use Wayland natively.
- On **Ubuntu 25.04** (and many recent Ubuntu/GNOME setups), the default session is **Wayland**. X11 apps then run under **XWayland**. In some environments (especially with certain compositors or in VMs), the FreeGLUT window can:
  - Be created but appear **transparent** or **invisible**
  - Fail to map or draw correctly, so you only see the launcher/icon
- In **Gnome Boxes** (or other VMs), **GPU/OpenGL** support is often limited or software-rendered. That can cause GL/GLX issues and result in a window that never draws, while the process and its dock icon still appear.

So the combination **Ubuntu 25.04 + Wayland + (optionally) Gnome Boxes** is a common scenario for “icon but no window”.

## What to try

### 1. Run from a terminal and check for errors

Run QB64-PE from a terminal so you can see any GLUT/OpenGL/X11 messages:

```bash
cd /path/to/your/QB64pe
./qb64pe
```

Note any errors (e.g. GLX, “BadValue”, “display mode not possible”, or OpenGL errors). Those can confirm display/GPU issues.

### 2. Use an X11 session (recommended if you’re on Wayland)

Log out, and at the login screen choose **“Ubuntu on Xorg”** (or “X11”) instead of the default Wayland session, then log in and run QB64-PE again. Many “icon but no window” cases are fixed by running under a plain X11 session.

### 3. Ensure XWayland is installed (if staying on Wayland)

If you stay on Wayland, XWayland must be present so X11 apps can run:

```bash
sudo apt install xwayland
```

Then try launching QB64-PE again. This does not always fix invisible windows, but it’s a prerequisite.

### 4. Try running with explicit display (X11/XWayland)

If you’re on Wayland and the compositor has started XWayland, you can try forcing the app to use the X11 display:

```bash
DISPLAY=:0 ./qb64pe
```

Or run via your launcher script:

```bash
DISPLAY=:0 ./run_qb64pe.sh
```

If the window was failing to connect to the right display, this can help. If the problem is transparency/rendering under XWayland, it may not.

### 5. Virtualization (Gnome Boxes / VMs)

If you’re inside **Gnome Boxes** or another VM:

- **3D/GPU acceleration** in the VM can be flaky. Enabling it (e.g. in the VM settings) may help, or you may need to test on bare metal or a different VM setup.
- **Software rendering** (e.g. llvmpipe) can be slow or buggy with OpenGL; that can also lead to a window that never draws.
- Testing QB64-PE on the **host** (same Ubuntu version, same Wayland vs X11) helps you see if the issue is specific to the VM.

## Summary

| Situation                         | Suggestion                                              |
|----------------------------------|---------------------------------------------------------|
| Ubuntu (or other distro) on Wayland | Prefer **“Ubuntu on Xorg”** (X11) at login, then run QB64-PE. |
| Already on X11                   | Run from terminal, check for GLX/OpenGL errors; try another machine or VM. |
| Running inside Gnome Boxes/VM    | Ensure XWayland is installed; try enabling 3D/GPU in VM; compare with host. |

The most reliable fix in many cases is to use an **X11 session** (e.g. “Ubuntu on Xorg”) when you need to run QB64-PE.

## Related documentation

- [WINDOW-MANAGEMENT.md](WINDOW-MANAGEMENT.md) — How QB64-PE creates and manages windows (GLUT/FreeGLUT).
- [SETUP-LNX.md](SETUP-LNX.md) — Linux setup and `run_qb64pe.sh`.
