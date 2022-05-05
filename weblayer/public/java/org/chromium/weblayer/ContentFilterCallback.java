// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer;

/**
 * Callback used to listen for content filter changes.
 */
public abstract class ContentFilterCallback {
    public abstract void onContentFilterStatsUpdated();
}
