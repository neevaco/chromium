// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/content_filter_manager_impl.h"

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_context.h"

#if BUILDFLAG(IS_ANDROID)
#include "base/android/jni_string.h"
#include "weblayer/browser/java/jni/ContentFilterManagerImpl_jni.h"
#endif

namespace weblayer {

ContentFilterManagerImpl::ContentFilterManagerImpl(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {}
ContentFilterManagerImpl::~ContentFilterManagerImpl() = default;

#if BUILDFLAG(IS_ANDROID)
void ContentFilterManagerImpl::GenerateRulesFile(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& input_file,
    const base::android::JavaParamRef<jobject>& callback) {
  // XXX
}

void ContentFilterManagerImpl::SetRulesFile(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& rules_file) {
//  SetRulesFile(base::FilePath(ConvertJavaStringToUTF8(rules_file)));
}

void ContentFilterManagerImpl::SetMode(JNIEnv* env, int mode) {
}

void ContentFilterManagerImpl::AddHostExclusion(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& host) {
//  AddHostExclusion(ConvertJavaStringToUTF8(host));
}

void ContentFilterManagerImpl::RemoveHostExclusion(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& host) {
//  RemoveHostExclusion(ConvertJavaStringToUTF8(host));
}

void ContentFilterManagerImpl::ClearAllHostExclusions(JNIEnv* env) {
//  ClearAllHostExclusions();
}

void ContentFilterManagerImpl::StartFiltering(JNIEnv* env) {
//  StartFiltering();
}

void ContentFilterManagerImpl::StopFiltering(JNIEnv* env) {
//  StopFiltering();
}
#endif

}  // namespace weblayer
