#!/bin/sh

# ----------先退回根目录----------
cd ..

APPKEY=lcaq6253mwcfrn3wri7g34ip7grx6hbhr65fiza6
ENABLE_OFFLINE=false
DEBUG_ON=true
ROOTDIR=$PWD

#------------------------------------------------------------start------------------------------------------------------------

Version=$(grep -oP 'OralSdkVersion *@"([0-9.]+)"' usc/Settings.h | grep -oP [0-9.]+)
Version="// V "$Version
echo $Version

rm -r OralEvalSDK_iOS
rm OralEvalSDK_iOS.zip
rm AsrOral.ipa


# ----------
# - Replace AppKey and close debug log -
cd usc
sed -i Settings.h "s/AppKey @\".*/AppKey @\"$APPKEY\"/" Settings.h


if $DEBUG_ON; then
	sed -i Settings.h "s/USC_BUG 0/USC_BUG 1/" Settings.h
else
	sed -i Settings.h "s/USC_BUG 1/USC_BUG 0/" Settings.h
fi


# ----------
# - Replace Recognize Mode (1:Mix 0:Online_Only) -
if $ENABLE_OFFLINE; then
	sed -i Settings.h "s/MIX_RECOGNIZE 0/MIX_RECOGNIZE 1/" Settings.h
else
	sed -i Settings.h "s/MIX_RECOGNIZE 1/MIX_RECOGNIZE 0/" Settings.h
fi

cd ..

# ----------
# - build libusc.a iphoneos -
xcodebuild -project usc/usc.xcodeproj clean
xcodebuild -project usc/usc.xcodeproj -target usc -configuration "Release" -sdk iphoneos DEPLOYMENT_POSTPROCESSING=YES


# ----------
# - copy libusc.a iphoneos -
mkdir -v -p OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos
cp -r libusc.a OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos


# ----------
# - copy USCRecognizer.h iphoneos -
cp -r usc/USCRecognizer.h OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos
sed -i '' -e "8i \\
$Version" OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos/USCRecognizer.h


# ----------
# - build libusc.a iphonesimulator -
xcodebuild -project usc/usc.xcodeproj clean
xcodebuild -project usc/usc.xcodeproj -target usc -configuration "Release" -sdk iphonesimulator DEPLOYMENT_POSTPROCESSING=YES


# ----------
# - copy libusc.a iphonesimulator -
mkdir -v -p OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphonesimulator
cp -r libusc.a OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphonesimulator


# ----------
# - copy USCRecognizer.h iphonesimulator -
cp -r usc/USCRecognizer.h OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphonesimulator
sed -i '' -e "8i \\
$Version" OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphonesimulator/USCRecognizer.h


# ----------
# - copy Resource for iphoneos and iphonesimulator -
if $ENABLE_OFFLINE; then
	cp -r usc/Recognition/Offline/OralEngine/tmp_oral_offline.bundle OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos
	cp -r usc/Recognition/Offline/OralEngine/tmp_oral_offline.bundle OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphonesimulator
fi


# ----------
# - copy docs -
cp -r Docs/Release/ OralEvalSDK_iOS/Docs/


# ----------
# - copy demo -
cp -r pro/Demo_Release/ OralEvalSDK_iOS/Demo


# ----------
# - copy lib files
rm  -r OralEvalSDK_iOS/Demo/AsrOralDemo/lib
cp -r -f OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos/ OralEvalSDK_iOS/Demo/AsrOralDemo/lib


# ----------
# - prepare -
cd OralEvalSDK_iOS/Demo
rm -r AsrOral.xcarchive
rm AsrOral.ipa


# ----------
# - clean proj -
xcodebuild clean -project AsrOralDemo.xcodeproj -alltargets


# ----------
# -  archive -
#path=/Users/cszhan/Desktop/work_new/AsrOral/Code/OralGit/oraleval-sdk-manager/ios-sdk/src/iOS/OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos
path=/OralEvalSDK_iOS/OralEvalSDK_Libs/Release-iphoneos
xcodebuild archive -project AsrOralDemo.xcodeproj -scheme AsrOralDemo -archivePath AsrOral.xcarchive LIBRARY_SEARCH_PATHS=$ROOTDIR$path


# ----------
# -  ipa -
xcodebuild -exportArchive -archivePath AsrOral.xcarchive -exportPath AsrOral.ipa -exportFormat ipa -exportProvisioningProfile AsrOral_adhoc


# ----------
# -  remove build file -
rm -r AsrOral.xcarchive
rm -r build
rm -r AsrOralDemo/lib


# ----------
# - move test Ipa -
mv AsrOral.ipa ../../AsrOral.ipa

# ----------
# - back to work dir
cd ../..


# ----------
# - zip
rm OralEvalSDK_iOS.zip
zip -r OralEvalSDK_iOS.zip OralEvalSDK_iOS/

# ----------
# - clean tmp
rm -rf OralEvalSDK_iOS/ 