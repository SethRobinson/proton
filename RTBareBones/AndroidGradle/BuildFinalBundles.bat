echo Set JAVA_HOME and other vars (hope you edited ../../base_setup.bat)
cd ..
call app_info_setup.bat
cd AndroidGradle

call gradlew bundle
pause