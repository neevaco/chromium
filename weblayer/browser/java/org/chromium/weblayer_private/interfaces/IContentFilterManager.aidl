// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer_private.interfaces;

/**
 * Controls per-profile content filtering rules.
 */
interface IContentFilterManager {
  void setRulesFile(in String rulesFile) = 1;
  void setMode(in int mode) = 2;
  void addHostExclusion(in String hostExclusion) = 3;
  void removeHostExclusion(in String hostExclusion) = 4;
  void clearAllHostExclusions() = 5;
  void startFiltering() = 6;
  void stopFiltering() = 7;
}
