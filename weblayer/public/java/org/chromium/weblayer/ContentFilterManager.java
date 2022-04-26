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
     * Generate a binary representation of the Adblock Plus 1.1 format rules file.
     *
     * @param inputFile the text file specifying the filter rules.
     * @param callback receives a file intended to be stored by the application
     * and used later via setRulesFile.
     */
    public void generateRulesFile(@NonNull File inputFile, @NonNull Callback<File> callback) {
        ThreadCheck.ensureOnUiThread();
        try {
            ValueCallback<String> valueCallback = (String result) -> {
                callback.onResult(new File(result));
            };
            mImpl.generateRulesFile(inputFile.toString(), ObjectWrapper.wrap(valueCallback));
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Sets the given rules file as the active rules file.
     */
    void setRulesFile(File rulesFile) {
        try {
            mImpl.setRulesFile(rulesFile.toString());
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Sets the mode for applying the rules.
     */
    void setMode(@ContentFilterMode int mode) {
        try {
            mImpl.setMode(mode);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Adds a host exclusion.
     */
    void addHostExclusion(String hostExclusion) {
        try {
            mImpl.addHostExclusion(hostExclusion);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Removes a host exclusion.
     */
    void removeHostExclusion(String hostExclusion) {
        try {
            mImpl.removeHostExclusion(hostExclusion);
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Clears all host exclusions.
     */
    void clearAllHostExclusions() {
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
    void startFiltering() {
        try {
            mImpl.startFiltering();
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Stops filtering.
     */
    void stopFiltering() {
        try {
            mImpl.stopFiltering();
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }
}
