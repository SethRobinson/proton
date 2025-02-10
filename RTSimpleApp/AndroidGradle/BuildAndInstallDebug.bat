echo Set JAVA_HOME and other vars (hope you edited ../../base_setup.bat)
cd ..
call app_info_setup.bat
cd AndroidGradle

:First uninstall any release builds, otherwise this will fail
call gradlew uninstallRelease
call gradlew installDebug
pause