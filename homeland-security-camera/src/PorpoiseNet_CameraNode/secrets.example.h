/* =====================================================================
 * secrets.example.h  --  the template. This file is safe to commit.
 * ---------------------------------------------------------------------
 * HOW TO USE IT
 *   1. Copy this file, in the same folder, and name the copy secrets.h
 *   2. Put the real values in secrets.h
 *   3. Never commit secrets.h. .gitignore already blocks it - do not
 *      add an exception, and do not rename it to get around the block.
 *
 * WHY THE TWO-FILE DANCE
 *   THIS REPOSITORY IS PUBLIC. Anyone on the internet can read every
 *   file in it and its entire history. A password typed into a committed
 *   file is published the moment it is pushed, and deleting it later
 *   does not unpublish it - the old commit still has it. That has
 *   already happened once in this repository (see the warning in
 *   ground-vehicle/README.md), which is why this pattern exists.
 *
 *   This example file is committed so that the next person can see WHICH
 *   settings exist without ever seeing what they are.
 * ===================================================================== */

#ifndef SECRETS_H
#define SECRETS_H

// The 2.4 GHz network the camera uploads over. ESP32 cannot use 5 GHz.
// Remember: joining a router fixes this board's radio channel, and every
// other PorpoiseNet board must then use that same channel. See the
// channel trap at the top of PorpoiseNet_CameraNode.ino.
#define WIFI_SSID      "your-network-name"
#define WIFI_PASSWORD  "your-network-password"

// Where captured images are POSTed, as raw image/jpeg. A server you
// control - not a third-party image host. Pictures from a school camera
// should not be handed to a company nobody has an agreement with.
//
// Put the real address in secrets.h, never here. This repository's rules
// treat an IP address as sensitive in its own right, because it maps out
// a network for anyone reading - which is why even the example below is
// not a real-looking address.
#define UPLOAD_URL     "http://your-server-address:8000/upload"

#endif  // SECRETS_H
