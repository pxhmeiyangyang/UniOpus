cd ../usc
grep -oP 'OralSdkVersion *@"([0-9.]+)"' ios-sdk/src/iOS/usc/Settings.h | grep -oP [0-9.]+ >> iOS_SDK_Version

ver="// V 2.16.10"
echo $ver

sed -i '' -e "8i \\
$ver" USCRecognizer.h

cd ..
Version=$(grep -oP 'OralSdkVersion *@"([0-9.]+)"' usc/Settings.h | grep -oP [0-9.]+)
Version="Last // V "$Version
echo $Version