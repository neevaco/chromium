// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * @hide
 */
@IntDef({ContentFilterMode.BLOCK_COOKIES, ContentFilterMode.BLOCK_REQUESTS})
@Retention(RetentionPolicy.SOURCE)
public @interface ContentFilterMode {
    /** The cookie was inserted. */
    int BLOCK_COOKIES = org.chromium.weblayer_private.interfaces.ContentFilterMode.BLOCK_COOKIES;
    /** The cookie was changed directly by a consumer's action. */
    int BLOCK_REQUESTS = org.chromium.weblayer_private.interfaces.ContentFilterMode.BLOCK_REQUESTS;
}
