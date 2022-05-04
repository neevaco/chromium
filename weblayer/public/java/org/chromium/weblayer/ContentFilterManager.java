// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer;

import android.os.RemoteException;
import android.webkit.ValueCallback;

import androidx.annotation.NonNull;

import org.chromium.weblayer_private.interfaces.APICallException;
import org.chromium.weblayer_private.interfaces.IContentFilterManager;
import org.chromium.weblayer_private.interfaces.IProfile;
import org.chromium.weblayer_private.interfaces.ObjectWrapper;

import java.io.File;

public class ContentFilterManager {
    private final IContentFilterManager mImpl;

    static ContentFilterManager create(IProfile profile) {
        try {
            return new ContentFilterManager(profile.getContentFilterManager());
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    private ContentFilterManager(IContentFilterManager impl) {
        mImpl = impl;
    }

    /**
     * Sets the given rules file as the active rules file.
     * @param apkPath the relative path into the APK where the rules file can
     * be found (e.g., "assets/easyprivacy.proto").
     */
    public void setRulesFile(String apkPath) {
        try {
            mImpl.setRulesFile(apkPath);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Sets the mode for applying the rules.
     */
    public void setMode(@ContentFilterMode int mode) {
        try {
            mImpl.setMode(mode);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Adds a host exclusion.
     * @param hostExclusion the hostname to exclude. Should be normalized (i.e., lowercase).
     */
    public void addHostExclusion(String hostExclusion) {
        try {
            mImpl.addHostExclusion(hostExclusion);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Removes a host exclusion.
     */
    public void removeHostExclusion(String hostExclusion) {
        try {
            mImpl.removeHostExclusion(hostExclusion);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Clears all host exclusions.
     */
    public void clearAllHostExclusions() {
        try {
            mImpl.clearAllHostExclusions();
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Starts filtering.
     * Use ContentFilterCallback to observe when filtering happens in a Tab.
     */
    public void startFiltering() {
        try {
            mImpl.startFiltering();
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Stops filtering.
     */
    public void stopFiltering() {
        try {
            mImpl.stopFiltering();
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }
}
