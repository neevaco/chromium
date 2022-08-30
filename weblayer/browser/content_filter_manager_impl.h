// Copyright 2022 Neeva. All rights reserved.

#ifndef WEBLAYER_BROWSER_CONTENT_FILTER_MANAGER_IMPL_H_
#define WEBLAYER_BROWSER_CONTENT_FILTER_MANAGER_IMPL_H_

#include "base/memory/raw_ptr.h"

#if BUILDFLAG(IS_ANDROID)
#include <jni.h>
#include "base/android/scoped_java_ref.h"
#endif

namespace content {
class BrowserContext;
}

namespace weblayer {

// Just exists as a helper to route JNI calls to ContentFilterRulesConfig.
class ContentFilterManagerImpl {
 public:
  explicit ContentFilterManagerImpl(content::BrowserContext* browser_context);
  ~ContentFilterManagerImpl();

#if BUILDFLAG(IS_ANDROID)
  void EnableRulesFile(
      JNIEnv* env, const base::android::JavaParamRef<jstring>& rules_file);
  void DisableAllRulesFiles(JNIEnv* env);
  void SetMode(JNIEnv* env, int mode);
  void AddHostExclusion(
      JNIEnv* env, const base::android::JavaParamRef<jstring>& host);
  void RemoveHostExclusion(
      JNIEnv* env, const base::android::JavaParamRef<jstring>& host);
  void ClearAllHostExclusions(JNIEnv* env);
  void StartFiltering(JNIEnv* env);
  void StopFiltering(JNIEnv* env);
#endif

 private:
  raw_ptr<content::BrowserContext> browser_context_;
};

}  // namespace weblayer

#endif  // WEBLAYER_BROWSER_NEEVA_CONTENT_FILTER_MANAGER_IMPL_H_
