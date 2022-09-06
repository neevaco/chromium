// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer_private.interfaces;

/**
 * Controls per-profile content filtering rules.
 */
interface IContentFilterManager {
  void enableRulesFile(in String rulesFile) = 1;
  void disableAllRulesFiles() = 2;
  void setMode(in int mode) = 3;
  void addHostExclusion(in String hostExclusion) = 4;
  void removeHostExclusion(in String hostExclusion) = 5;
  void clearAllHostExclusions() = 6;
  void startFiltering() = 7;
  void stopFiltering() = 8;
}
