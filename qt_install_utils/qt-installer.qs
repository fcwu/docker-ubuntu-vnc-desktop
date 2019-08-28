// installer script for the Qt installer. Launch with (-platform is for headless)
// ./installer -platform minimal --script path/to/qt-installer.qs

function Controller() {
    installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton);
    })
}

Controller.prototype.WelcomePageCallback = function() {
    // click delay here because the next button is initially disabled for ~1 second
    gui.clickButton(buttons.NextButton, 3000);
}

Controller.prototype.CredentialsPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    gui.currentPageWidget().TargetDirectoryLineEdit.setText("/opt/Qt5.12.4");
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    var widget = gui.currentPageWidget();
    widget.deselectAll();
    // Package names are a mystery but can be extracted using the following script with a --list-packages flag:
    // https://raw.githubusercontent.com/benlau/qtci/master/bin/extract-qt-installer
    // Here is the full list for 5.12:
    // qt qt.qt5.5124 qt.tools qt.installer.changelog qt.license.gplv3except qt.license.python qt.license.thirdparty qt.license.lgpl qt.qt5.5124.gcc_64 qt.qt5.5124.android_x86 qt.qt5.5124.android_arm64_v8a qt.qt5.5124.android_armv7 qt.qt5.5124.src qt.qt5.5124.qtcharts qt.qt5.5124.qtdatavis3d qt.qt5.5124.qtpurchasing qt.qt5.5124.qtvirtualkeyboard qt.qt5.5124.qtwebengine qt.qt5.5124.qtnetworkauth qt.qt5.5124.qtwebglplugin qt.qt5.5124.qtscript qt.qt5.5124.examples qt.qt5.5124.doc qt.qt5.5124.qtcharts.android_x86 qt.qt5.5124.qtcharts.android_arm64_v8a qt.qt5.5124.qtcharts.android_armv7 qt.qt5.5124.qtcharts.gcc_64 qt.qt5.5124.qtdatavis3d.android_x86 qt.qt5.5124.qtdatavis3d.android_arm64_v8a qt.qt5.5124.qtdatavis3d.android_armv7 qt.qt5.5124.qtdatavis3d.gcc_64 qt.qt5.5124.qtpurchasing.android_arm64_v8a qt.qt5.5124.qtpurchasing.android_x86 qt.qt5.5124.qtpurchasing.gcc_64 qt.qt5.5124.qtpurchasing.android_armv7 qt.qt5.5124.qtvirtualkeyboard.gcc_64 qt.qt5.5124.qtwebengine.gcc_64 qt.qt5.5124.qtnetworkauth.android_x86 qt.qt5.5124.qtnetworkauth.gcc_64 qt.qt5.5124.qtnetworkauth.android_armv7 qt.qt5.5124.qtnetworkauth.android_arm64_v8a qt.qt5.5124.qtwebglplugin.gcc_64 qt.qt5.5124.qtscript.android_armv7 qt.qt5.5124.qtscript.android_arm64_v8a qt.qt5.5124.qtscript.android_x86 qt.qt5.5124.qtscript.gcc_64 qt.qt5.5124.examples.qtdatavis3d qt.qt5.5124.examples.qtpurchasing qt.qt5.5124.examples.qtcharts qt.qt5.5124.examples.qtwebengine qt.qt5.5124.examples.qtscript qt.qt5.5124.examples.qtnetworkauth qt.qt5.5124.examples.qtvirtualkeyboard qt.qt5.5124.doc.qtwebengine qt.qt5.5124.doc.qtpurchasing qt.qt5.5124.doc.qtdatavis3d qt.qt5.5124.doc.qtcharts qt.qt5.5124.doc.qtvirtualkeyboard qt.qt5.5124.doc.qtnetworkauth qt.qt5.5124.doc.qtscript qt.tools.qtcreator
    // qt qt.qt5.5124 qt.tools qt.installer.changelog qt.license.gplv3except qt.license.python qt.license.thirdparty qt.license.lgpl qt.qt5.5124.gcc_64 qt.qt5.5124.android_x86 qt.qt5.5124.android_arm64_v8a qt.qt5.5124.android_armv7 qt.qt5.5124.src qt.qt5.5124.qtcharts qt.qt5.5124.qtdatavis3d qt.qt5.5124.qtpurchasing qt.qt5.5124.qtvirtualkeyboard qt.qt5.5124.qtwebengine qt.qt5.5124.qtnetworkauth qt.qt5.5124.qtwebglplugin qt.qt5.5124.qtscript ...
    widget.selectComponent("qt.qt5.5124.gcc_64");
    //widget.selectComponent("qt.qt5.5124.qtcharts");
    //widget.selectComponent("qt.qt5.5124.qtdatavis3d");
    //widget.selectComponent("qt.qt5.5124.qtvirtualkeyboard");
    //widget.selectComponent("qt.qt5.5124.qtwebengine");
    //widget.selectComponent("qt.qt5.5124.qtnetworkauth");
    //widget.selectComponent("qt.qt5.5124.qtwebglplugin");
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function() {
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function() {
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
}
