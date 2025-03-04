echo Set JAVA_HOME and other vars (hope you edited ../../base_setup.bat)
cd ..
call app_info_setup.bat
cd AndroidGradle

#to force a full rebuild
#call gradlew clean

:First uninstall any release builds, otherwise this will fail
call gradlew uninstallRelease
call gradlew installDebug

:: Launch the app automatically
adb shell monkey -p com.rtsoft.rtsimpleapp -c android.intent.category.LAUNCHER 1
pause