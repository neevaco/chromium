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
     * @param outputFile the path to the file that will be generated. This file can
     * be stored by the application and later passed setRulesFile.
     * @param callback receives a boolean intended if the generation succeeded.
     */
    public void generateRulesFile(@NonNull File inputFile, @NonNull File outputFile,
                                  @NonNull Callback<Boolean> callback) {
        ThreadCheck.ensureOnUiThread();
        try {
            ValueCallback<Boolean> valueCallback = (Boolean result) -> {
                callback.onResult(result);
            };
            mImpl.generateRulesFile(inputFile.toString(), outputFile.toString(),
                                    ObjectWrapper.wrap(valueCallback));
        } catch (RemoteException e) {
            throw new APICallException(e);
        }
    }

    /**
     * Sets the given rules file as the active rules file.
     */
    public void setRulesFile(File rulesFile) {
        try {
            mImpl.setRulesFile(rulesFile.toString());
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
