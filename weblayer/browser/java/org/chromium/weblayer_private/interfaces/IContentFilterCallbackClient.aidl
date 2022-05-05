// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer_private.interfaces;

/**
 * Indicates when ITab.getContentFilterStats would return new data.
 */
interface IContentFilterCallbackClient {
  void onContentFilterStatsUpdated() = 0;
}
