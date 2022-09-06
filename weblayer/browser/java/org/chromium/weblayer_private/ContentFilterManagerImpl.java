// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer_private;

import android.webkit.ValueCallback;

import org.chromium.base.Callback;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.weblayer_private.interfaces.ContentFilterMode;
import org.chromium.weblayer_private.interfaces.IContentFilterManager;
import org.chromium.weblayer_private.interfaces.IObjectWrapper;
import org.chromium.weblayer_private.interfaces.ObjectWrapper;
import org.chromium.weblayer_private.interfaces.StrictModeWorkaround;

/**
 * Implementation of IContentFilterManager.
 */
@JNINamespace("weblayer")
public final class ContentFilterManagerImpl extends IContentFilterManager.Stub {
    private long mNativeContentFilterManager;

    ContentFilterManagerImpl(long nativeContentFilterManager) {
        mNativeContentFilterManager = nativeContentFilterManager;
    }

    public void destroy() {
        mNativeContentFilterManager = 0;
    }

    @Override
    public void enableRulesFile(String rules_file) {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().enableRulesFile(mNativeContentFilterManager, rules_file);
    }

    @Override
    public void disableAllRulesFiles() {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().disableAllRulesFiles(mNativeContentFilterManager);
    }

    @Override
    public void setMode(int mode) {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().setMode(mNativeContentFilterManager, mode);
    }

    @Override
    public void addHostExclusion(String host) {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().addHostExclusion(mNativeContentFilterManager, host);
    }

    @Override
    public void removeHostExclusion(String host) {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().removeHostExclusion(mNativeContentFilterManager, host);
    }

    @Override
    public void clearAllHostExclusions() {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().clearAllHostExclusions(mNativeContentFilterManager);
    }

    @Override
    public void startFiltering() {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().startFiltering(mNativeContentFilterManager);
    }

    @Override
    public void stopFiltering() {
        StrictModeWorkaround.apply();
        ContentFilterManagerImplJni.get().stopFiltering(mNativeContentFilterManager);
    }

    @NativeMethods
    interface Natives {
        void enableRulesFile(long nativeContentFilterManagerImpl, String rulesFile);
        void disableAllRulesFiles(long nativeContentFilterManagerImpl);
        void setMode(long nativeContentFilterManagerImpl, int mode);
        void addHostExclusion(long nativeContentFilterManagerImpl, String host);
        void removeHostExclusion(long nativeContentFilterManagerImpl, String host);
        void clearAllHostExclusions(long nativeContentFilterManagerImpl);
        void startFiltering(long nativeContentFilterManagerImpl);
        void stopFiltering(long nativeContentFilterManagerImpl);
    }
}
