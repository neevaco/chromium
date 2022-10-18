#!/bin/sh

# This script is used to package up a release build.

out=$1
src=${2:-"out/build.release-android"}

if [ -z "$out" ]; then
	echo "Usage: $(basename $0) out-dir [src-dir]"
	exit 1
fi

echo "Taking snapshot of $src"
if [ -e "$out" ]; then 
  echo -n "$out already exists, hit enter to delete: "
  read continue_confirmation
fi

rm -rf $out
mkdir -p $out
cp $src/apks/WebLayerSupport.apk $out
cp $src/apks/WebLayerShell.apk $out
cp $src/args.gn $out

get_list_of_public_java_sources() {
  # Exclude unneeded browser sandbox code.
  find org/chromium/weblayer -name \*.java | egrep -v 'BrowserSandboxService.java|BrowserFragmentDelegate.java|BrowserFragmentTabDelegate.java|TabNavigationControllerProxy.java|TabParams.java|TabProxy.java|WebMessageReplyProxyProxy.java'
}

(cd $src/../../weblayer/public/java && zip -r $out/client-res.zip res)
(cd $src/../../weblayer/public/java && zip -r $out/client-java.zip $(get_list_of_public_java_sources))
(cd $src/../../weblayer/browser/java && zip -r $out/client-java.zip $(find org/chromium/weblayer_private/interfaces -name \*.java))
(cd $src/gen/weblayer/public/java && zip -r $out/client-java.zip org/chromium/weblayer/WebLayerClientVersionConstants.java)
(cd $src/../../weblayer/browser/java && zip -r $out/client-aidl.zip $(find org/chromium/weblayer_private/interfaces -name \*.aidl))
cp -f $src/gen/weblayer/public/java/weblayer_client_manifest/AndroidManifest.xml $out
cp -f $src/lib.unstripped/libweblayer_test.so.map.gz $out

echo "Done"
ls -latr $out
