# New Machine Setup Guide

Step-by-step instructions for setting up the development environment from scratch on a new machine. Follow this guide whenever you forget how everything fits together.

For project architecture, hardware pinout, module documentation, and task tracking, see `Software/PROJECT_GUIDE.md`.

---

## Prerequisites

Install these before anything else:

1. **nRF Connect for Desktop** — download from [nordicsemi.com](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop). Use it to install the **Toolchain Manager** and download **NCS v3.2.4**.
   - After installing, the toolchain will be at: `/home/christian/ncs/v3.2.4/`
2. **VS Code** — with the **nRF Connect Extension Pack** (Nordic Semiconductor) installed from the Extensions marketplace.

---

## Opening the Project

Always open the workspace file, not just the folder:

```
File → Open Workspace from File → Mouse.code-workspace
```

Opening the workspace activates the `nrf-connect.boardRoots` and `nrf-connect.applications` settings that tell the extension where to find the custom board definitions and application projects.

---

## Configuring the nRF Connect Extension

The first time you open the workspace on a new machine:

1. Open the **nRF Connect** sidebar (Nordic icon in the Activity Bar)
2. Under **Toolchain**, select **NCS v3.2.4** — it should auto-detect from `/home/christian/ncs/v3.2.4/`
3. If not detected, click "Manage toolchains" and point it at the correct path

---

## Registering the Custom Board Root

The nice!nano board definitions live in this repo at `Software/boards/` and are not part of the NCS SDK installation. The extension must be told where to find them.

The workspace file (`Mouse.code-workspace`) already includes the board root setting:

```json
"nrf-connect.boardRoots": [
  "/home/christian/Projects/Mouse/Software"
]
```

**If the repo is cloned to a different path**, update this setting in one of two ways:
- Edit `Mouse.code-workspace` and change the path to match your clone location
- Or go to VS Code **Settings → Extensions → nRF Connect → Board Roots** and add the path to your `Software/` directory there

The board root must be the `Software/` directory (the one that *contains* `boards/`), not the `boards/` directory itself.

---

## Verifying Boards Appear

1. In the nRF Connect sidebar, click **Add Build Configuration**
2. In the **Board** field, search for `nice_nano`
3. You should see:
   - `nice_nano/nrf52840` — nicekeyboards port (supports v1.0.0 and v2.0.0 revisions)
   - `nice_nano_v2/nrf52840` — aliexpress port

If neither appears, the board root is not registered correctly. Double-check the path in `Mouse.code-workspace` and reload VS Code (`Ctrl+Shift+P → Reload Window`).

---

## Creating a Build Configuration

In the nRF Connect sidebar:

1. Click **Add Build Configuration**
2. **Application:** select `nrf_desktop-2026-04-23` (this is the active project)
3. **Board:** `nice_nano/nrf52840`
4. **Build directory:** leave as default or use a name like `build_nicenano`
5. Click **Build**

Build artifacts appear in `build_nicenano/zephyr/`:
- `zephyr.hex` — the firmware binary
- `zephyr.dts` — resolved device tree (useful for debugging)

---

## Flashing to the nice!nano (UF2)

The nice!nano uses a UF2 drag-and-drop bootloader. No J-Link required.

1. **Build** the project (step above)
2. **Convert to UF2:**
   ```bash
   uf2conv.py build_nicenano/zephyr/zephyr.hex -c -f 0xADA52840
   ```
   This creates `flash.uf2` in the current directory.
3. **Enter bootloader mode:** double-tap the RESET button on the nice!nano. A USB drive named `NICENANO` will appear.
4. **Copy** `flash.uf2` to the `NICENANO` drive. The board auto-flashes and reboots.

UF2 family IDs for reference:
- nRF52840 firmware: `0xADA52840`
- nRF52833 firmware: `0x621E937A`
- Bootloader update: `0xd663823c`

---

## CLI Build Alternative

If the VS Code extension is not working, build from the terminal using absolute paths:

```bash
west build \
  --build-dir /home/christian/Projects/Mouse/Software/build_nicenano \
  /home/christian/Projects/Mouse/Software/nrf_desktop-2026-04-23 \
  -- \
  -DBOARD=nice_nano/nrf52840 \
  -DBOARD_ROOT=/home/christian/Projects/Mouse/Software
```

The `-DBOARD_ROOT` argument is the CLI equivalent of `nrf-connect.boardRoots`.

---

## About the Archive Subdirectories

The `nice_nano/` and `nice_nano_nrf52840/` subdirectories in this `Guide/` folder are **dated snapshots** of known-good files:

| Directory | What it contains | Where to put these files |
|-----------|-----------------|--------------------------|
| `nice_nano/` | Board definition files (DTS, Kconfig, overlays) | `Software/boards/nicekeyboards/nice_nano/` |
| `nice_nano_nrf52840/` | Application config files (prj.conf, app.overlay, *_def.h) | `Software/nrf_desktop-x/configuration/nice_nano_nrf52840/` |

Use them to restore a known-good state if experimenting with configuration changes breaks something. The timestamp in the `*.md` file inside each subdirectory indicates when the snapshot was taken.
