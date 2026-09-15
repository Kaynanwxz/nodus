# Nodus — Design Direction

## Status

**Approved visual direction — 2026-09-15**

This document is the visual source of truth for the Nodus interface until a newer direction is explicitly approved.

## Core identity

Nodus should feel like a **dark classical surrealist collage transformed into a usable desktop application**.

It must not look cyberpunk, neon-heavy, holographic, sci-fi generic, or like a standard SaaS dashboard.

The visual references are closer to:

- antique engravings;
- Renaissance/classical sculpture;
- anatomical drawings;
- occult/celestial diagrams;
- old maps and manuscripts;
- torn paper collage;
- editorial art direction;
- museum/archive aesthetics;
- dark romanticism and surrealism.

## Color palette

Primary environment:

- near black;
- charcoal;
- smoke gray;
- deep brown/sepia.

Accent colors:

- muted gold;
- ochre;
- aged ivory;
- parchment beige;
- restrained green for positive/running state;
- restrained amber for working/warning state;
- restrained red only where status requires it.

Avoid saturated neon colors.

## Application frame

The native white Windows title bar must **not** be visible.

The target is a custom-drawn/frameless Windows surface where the application begins visually with the Nodus header.

The custom header remains part of the approved design.

### Header content

Left:

- `NODUS`
- small identity/tagline beneath it.

Center:

- `Overview`
- `Agents`
- `Terminal`
- `Tasks`
- `Activity`
- `Settings`

Right:

- search;
- lightweight status/system indicator;
- time if useful;
- optional profile/identity medallion.

The selected tab may use a torn-paper/ochre treatment, but it should remain elegant and compact.

## Hero area

The home dashboard should have a strong editorial composition before the data panels.

Approved language:

- large serif Nodus statement/headline;
- classical figure or bust on the left;
- angel/wing/hand composition near center-right;
- engraved eye / celestial diagram / map details;
- parchment fragments containing short phrases;
- collage elements framing the dashboard rather than obscuring information.

The art should be rich around the edges and calmer behind interactive data.

## Dashboard structure

### Metrics

A row of compact dark cards for information such as:

- agents online;
- tasks running;
- total actions;
- system health.

Use thin warm borders and minimal iconography.

### Agents

Agent cards should show:

- portrait/engraving medallion;
- name;
- role;
- short description/current task;
- status;
- task count or other small metadata.

Agent portraits should reinforce the classical/collage language instead of using generic colored avatars.

### Tasks

Dense but readable list/table.

Show:

- task name;
- assigned agent;
- status;
- progress/time;
- compact action menu.

### Activity

Vertical event list with restrained icons and timestamps.

It should resemble an archive/logbook more than a social media feed.

## Typography

Use an editorial combination:

- serif display type for Nodus branding, headings and artistic statements;
- restrained serif or highly readable UI face for content;
- monospaced type only where terminal/technical data benefits from it.

Avoid making the entire application look like a terminal.

## Texture and imagery

Preferred motifs:

- eyes;
- classical busts;
- hands;
- angels;
- skull/anatomy details;
- butterflies/moths;
- architectural ruins;
- celestial charts;
- handwriting;
- roses/botanical fragments;
- torn paper.

Texture must add character without reducing text legibility.

## Performance constraint

Visual richness cannot turn Nodus into a heavy application.

The implementation remains native C/Win32. Artwork should be optimized and reused rather than relying on a web runtime.

Possible implementation direction:

- custom non-client window frame;
- GDI/GDI+ or another lightweight native drawing path;
- compressed local image assets;
- cached bitmaps;
- avoid unnecessary animation;
- no Electron or embedded browser.

## Non-goals

Do not drift toward:

- futuristic neon dashboards;
- blue/purple cyberpunk gradients;
- generic glassmorphism;
- enterprise SaaS visuals;
- huge empty card layouts;
- default Windows controls that visually break the composition;
- default white Windows title bar.

## Current implementation priority

1. Custom frameless window/chrome.
2. Approved top navigation.
3. Dashboard layout matching the selected collage concept.
4. Optimized visual assets.
5. Only after the shell is visually stable, continue deeper agent functionality.
