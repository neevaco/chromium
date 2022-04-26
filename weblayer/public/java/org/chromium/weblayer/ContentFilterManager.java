// Copyright 2022 Neeva. All rights reserved.

package org.chromium.weblayer;

import android.webkit.ValueCallback;

import org.chromium.weblayer_private.interfaces.IContentFilterManager;

import java.io.File;

public class ContentFilterManager {
    private final IContentFilterManager mImpl;

    /**
     * Generate a binary representation of the Adblock Plus 1.1 format rules file.
     *
     * @param inputFile the text file specifying the filter rules.
     * @param callback receives a file intended to be stored by the application
     * and used later via setRulesFile.
     */
    public void generateRulesFile(File inputFile, ValueCallback<File> callback) {
    }

    /**
     * Sets the given rules file as the active rules file.
     */
    void setRulesFile(File rulesFile) {
    }

    /**
     * Sets the mode for applying the rules.
     */
    void setMode(ContentFilterMode mode) {
    }

    /**
     * Adds a host exclusion.
     */
    void addHostExclusion(String hostExclusion) {
    }

    /**
     * Removes a host exclusion.
     */
    void removeHostExclusion(String hostExclusion) {
    }

    /**
     * Clears all host exclusions.
     */
    void clearAllHostExclusions() {
    }

    /**
     * Starts filtering.
     * Use ContentFilterCallback to observe when filtering happens in a Tab.
     */
    void startFiltering() {
    }

    /**
     * Stops filtering.
     */
    void stopFiltering() {
    }
}
