// Copyright 2020 Neeva. All rights reserved.

package org.chromium.weblayer_private.interfaces;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

@IntDef({ContentFilterMode.BLOCK_COOKIES, ContentFilterMode.BLOCK_REQUESTS})
@Retention(RetentionPolicy.SOURCE)
public @interface ContentFilterMode {
    int BLOCK_COOKIES = 0;
    int BLOCK_REQUESTS = 1;
}
