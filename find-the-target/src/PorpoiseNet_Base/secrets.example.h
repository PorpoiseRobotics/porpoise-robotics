/* =====================================================================
 * secrets.example.h  --  the template. This file is safe to commit.
 * ---------------------------------------------------------------------
 * HOW TO USE IT
 *   1. Copy this file, in the same folder, and name the copy secrets.h
 *   2. Put the real values in secrets.h
 *   3. Never commit secrets.h. .gitignore already blocks it - do not add
 *      an exception, and do not rename it to get around the block.
 *
 * WHY THE TWO-FILE DANCE
 *   THIS REPOSITORY IS PUBLIC. Anyone on the internet can read every file
 *   in it and its entire history. A password typed into a committed file
 *   is published the moment it is pushed, and deleting it later does not
 *   unpublish it - the old commit still has it. That has already happened
 *   once in this repository (see the warning in ground-vehicle/README.md),
 *   which is why this pattern exists.
 *
 *   This example file is committed so the next person can see WHICH
 *   settings exist without ever seeing what they are.
 * ===================================================================== */

#ifndef SECRETS_H
#define SECRETS_H

// The network the base station joins. It must be 2.4 GHz - an ESP32
// cannot see 5 GHz networks at all, and a 5 GHz-only SSID looks exactly
// like a wrong password from the board's point of view.
//
// REMEMBER: joining this network fixes the base station's radio channel,
// and every other PorpoiseNet board must then be on that same channel.
// The sketch prints the number at boot. See the channel note at the top
// of PorpoiseNet_Base.ino.
#define WIFI_SSID      "your-network-name"
#define WIFI_PASSWORD  "your-network-password"

#endif  // SECRETS_H
