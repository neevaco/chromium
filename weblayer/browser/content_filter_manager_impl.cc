// Copyright 2022 Neeva. All rights reserved.

#include "weblayer/browser/content_filter_manager_impl.h"

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/ptr_util.h"
#include "components/neeva/browser/content_filter_rules_config.h"
#include "content/public/browser/browser_context.h"

#if BUILDFLAG(IS_ANDROID)
#include "base/android/callback_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "weblayer/browser/java/jni/ContentFilterManagerImpl_jni.h"
#endif

namespace weblayer {
namespace {

neeva::mojom::ContentFilterMode ToContentFilterMode(int mode) {
  switch (mode) {
    case 0:
      return neeva::mojom::ContentFilterMode::BLOCK_COOKIES;
    default:
      return neeva::mojom::ContentFilterMode::BLOCK_REQUESTS;
  }
}

}  // namespace

ContentFilterManagerImpl::ContentFilterManagerImpl(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {}
ContentFilterManagerImpl::~ContentFilterManagerImpl() = default;

#if BUILDFLAG(IS_ANDROID)
void ContentFilterManagerImpl::SetRulesFile(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& apk_path) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->
      SetRulesFile(ConvertJavaStringToUTF8(apk_path));
}

void ContentFilterManagerImpl::SetMode(JNIEnv* env, int mode) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->
      SetMode(ToContentFilterMode(mode));
}

void ContentFilterManagerImpl::AddHostExclusion(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& host) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->
      AddHostExclusion(ConvertJavaStringToUTF8(host));
}

void ContentFilterManagerImpl::RemoveHostExclusion(
    JNIEnv* env, const base::android::JavaParamRef<jstring>& host) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->
      RemoveHostExclusion(ConvertJavaStringToUTF8(host));
}

void ContentFilterManagerImpl::ClearAllHostExclusions(JNIEnv* env) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->ClearAllHostExclusions();
}

void ContentFilterManagerImpl::StartFiltering(JNIEnv* env) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->StartFiltering();
}

void ContentFilterManagerImpl::StopFiltering(JNIEnv* env) {
  neeva::ContentFilterRulesConfig::GetOrCreate(browser_context_)->StopFiltering();
}
#endif

}  // namespace weblayer
