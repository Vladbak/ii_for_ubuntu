'use strict';

const electron = require('electron');
const app = electron.app;  // Module to control application life.
const BrowserWindow = electron.BrowserWindow;
const nativeImage = electron.nativeImage;

const notifier = require('node-notifier');
const util = require('util');
const dialog = electron.dialog;
const Menu = require('menu');
const Tray = require('tray');
const shell = require('shell');
const os = require('os');
const childProcess = require('child_process');
const path = require('path');
const fs = require('fs-extra');
const Tail = require('always-tail');
const http = require('http');
const unzip = require('unzip');

const request = require('request');
const progress = require('request-progress');
const osHomeDir = require('os-homedir');

const updater = require('./modules/hf-updater.js');

const Config = require('./modules/config').Config;

const hfprocess = require('./modules/hf-process.js');
const Process = hfprocess.Process;
const ACMonitorProcess = hfprocess.ACMonitorProcess;
const ProcessStates = hfprocess.ProcessStates;
const ProcessGroup = hfprocess.ProcessGroup;
const ProcessGroupStates = hfprocess.ProcessGroupStates;

const osType = os.type();

const appIcon = path.join(__dirname, '../resources/console.png');

const DELETE_LOG_FILES_OLDER_THAN_X_SECONDS = 60 * 60 * 24 * 7; // 7 Days
const LOG_FILE_REGEX = /(domain-server|ac-monitor|ac)-.*-std(out|err).txt/;

function getBuildInfo() {
    var buildInfoPath = null;

    if (osType == 'Windows_NT') {
        buildInfoPath = path.join(path.dirname(process.execPath), 'build-info.json');
    } else if (osType == 'Darwin') {
        var contentPath = ".app/Contents/";
        var contentEndIndex = __dirname.indexOf(contentPath);

        if (contentEndIndex != -1) {
            // this is an app bundle
            var appPath = __dirname.substring(0, contentEndIndex) + ".app";
            buildInfoPath = path.join(appPath, "/Contents/Resources/build-info.json");
        }
    }

    const DEFAULT_BUILD_INFO = { releaseType: "", buildIdentifier: "dev" };
    var buildInfo = DEFAULT_BUILD_INFO;

    if (buildInfoPath) {
        console.log('Build info path:', buildInfoPath);
        try {
            buildInfo = JSON.parse(fs.readFileSync(buildInfoPath));
        } catch (e) {
            buildInfo = DEFAULT_BUILD_INFO;
        }
    }

    return buildInfo;
}

const buildInfo = getBuildInfo();

console.log("build info", buildInfo);

function getRootHifiDataDirectory() {
    var organization = "High Fidelity";
    if (buildInfo.releaseType != "PRODUCTION") {
        organization += ' - ' + buildInfo.buildIdentifier;
    }
    if (osType == 'Windows_NT') {
        return path.resolve(osHomeDir(), 'AppData/Roaming', organization);
    } else if (osType == 'Darwin') {
        return path.resolve(osHomeDir(), 'Library/Application Support', organization);
    } else {
        return path.resolve(osHomeDir(), '.local/share/', organization);
    }
}

function getAssignmentClientResourcesDirectory() {
    return path.join(getRootHifiDataDirectory(), '/assignment-client');
}

function getApplicationDataDirectory() {
    return path.join(getRootHifiDataDirectory(), '/Server Console');
}

console.log("Root hifi directory is: ", getRootHifiDataDirectory());

const ipcMain = electron.ipcMain;


var isShuttingDown = false;
function shutdown() {
    if (!isShuttingDown) {
        // if the home server is running, show a prompt before quit to ask if the user is sure
        if (homeServer.state == ProcessGroupStates.STARTED) {
            dialog.showMessageBox({
                type: 'question',
                buttons: ['Yes', 'No'],
                title: 'Stopping Server Console',
                message: 'Quitting will stop your Server Console and your Home domain will no longer be running.\nDo you wish to continue?'
            }, shutdownCallback);
        } else {
            shutdownCallback(0);
        }

    }
}

function shutdownCallback(idx) {
    if (idx == 0 && !isShuttingDown) {
        isShuttingDown = true;

        userConfig.save(configPath);

        if (logWindow) {
            logWindow.close();
        }
        if (homeServer) {
            homeServer.stop();
        }

        updateTrayMenu(homeServer.state);

        if (homeServer.state == ProcessGroupStates.STOPPED) {
            // if the home server is already down, take down the server console now
            app.quit();
        } else {
            // if the home server is still running, wait until we get a state change or timeout
            // before quitting the app
            var timeoutID = setTimeout(app.quit, 5000);
            homeServer.on('state-update', function(processGroup) {
                if (processGroup.state == ProcessGroupStates.STOPPED) {
                    clearTimeout(timeoutID);
                    app.quit();
                }
            });
        }
    }
}

function deleteOldFiles(directoryPath, maxAgeInSeconds, filenameRegex) {
    console.log("Deleting old log files in " + directoryPath);

    var filenames = [];
    try {
        filenames = fs.readdirSync(directoryPath);
    } catch (e) {
        console.warn("Error reading contents of log file directory", e);
        return;
    }

    for (const filename of filenames) {
        console.log("Checking", filename);
        const absolutePath = path.join(directoryPath, filename);
        var stat = null;
        try {
            stat = fs.statSync(absolutePath);
        } catch (e) {
            console.log("Error stat'ing file", absolutePath, e);
            continue;
        }
        const curTime = Date.now();
        if (stat.isFile() && filename.search(filenameRegex) >= 0) {
            const ageInSeconds = (curTime - stat.mtime.getTime()) / 1000.0;
            if (ageInSeconds >= maxAgeInSeconds) {
                console.log("\tDeleting:", filename, ageInSeconds);
                try {
                    fs.unlinkSync(absolutePath);
                } catch (e) {
                    if (e.code != 'EBUSY') {
                        console.warn("\tError deleting:", e);
                    }
                }
            }
        }
    }
}

var logPath = path.join(getApplicationDataDirectory(), '/logs');

console.log("Log directory:", logPath);
console.log("Data directory:", getRootHifiDataDirectory());

const configPath = path.join(getApplicationDataDirectory(), 'config.json');
var userConfig = new Config();
userConfig.load(configPath);

// print out uncaught exceptions in the console
process.on('uncaughtException', function(err) {
    console.error(err);
    console.error(err.stack);
});

var shouldQuit = app.makeSingleInstance(function(commandLine, workingDirectory) {
    // Someone tried to run a second instance, focus the window (if there is one)
    return true;
});

if (shouldQuit) {
    console.warn("Another instance of the Server Console is already running - this instance will quit.");
    app.quit();
    return;
}

// Check command line arguments to see how to find binaries
var argv = require('yargs').argv;
var pathFinder = require('./modules/path-finder.js');

var interfacePath = null;
var dsPath = null;
var acPath = null;

var debug = argv.debug;

var binaryType = argv.binaryType;

interfacePath = pathFinder.discoveredPath("Interface", binaryType, buildInfo.releaseType);
dsPath = pathFinder.discoveredPath("domain-server", binaryType, buildInfo.releaseType);
acPath = pathFinder.discoveredPath("assignment-client", binaryType, buildInfo.releaseType);

function binaryMissingMessage(displayName, executableName, required) {
    var message = "The " + displayName + " executable was not found.\n";

    if (required) {
        message += "It is required for the Server Console to run.\n\n";
    } else {
        message += "\n";
    }

    if (debug) {
        message += "Please ensure there is a compiled " + displayName + " in a folder named build in this checkout.\n\n";
        message += "It was expected to be found at one of the following paths:\n";

        var paths = pathFinder.searchPaths(executableName, argv.localReleaseBuilds);
        message += paths.join("\n");
    } else {
        message += "It is expected to be found beside this executable.\n";
        message += "You may need to re-install the Server Console.";
    }

    return message;
}

// if at this point any of the paths are null, we're missing something we wanted to find

if (!dsPath) {
    dialog.showErrorBox("Domain Server Not Found", binaryMissingMessage("domain-server", "domain-server", true));
    app.quit();
}

if (!acPath) {
    dialog.showErrorBox("Assignment Client Not Found", binaryMissingMessage("assignment-client", "assignment-client", true));
    app.quit();
}

function openFileBrowser(path) {
    // Add quotes around path
    path = '"' + path + '"';
    if (osType == "Windows_NT") {
        childProcess.exec('start "" ' + path);
    } else if (osType == "Darwin") {
        childProcess.exec('open ' + path);
    } else if (osType == "Linux") {
        childProcess.exec('xdg-open ' + path);
    }
}

// NOTE: this looks like it does nothing, but it's very important.
// Without it the default behaviour is to quit the app once all windows closed
// which is absolutely not what we want for a taskbar application.
app.on('window-all-closed', function() {
});

function startInterface(url) {
    var argArray = [];

    // check if we have a url parameter to include
    if (url) {
        argArray = ["--url", url];
    }

    // create a new Interface instance - Interface makes sure only one is running at a time
    var pInterface = new Process('interface', interfacePath, argArray);
    pInterface.start();
}

var tray = null;
global.homeServer = null;
global.domainServer = null;
global.acMonitor = null;
global.userConfig = userConfig;

var LogWindow = function(ac, ds) {
    this.ac = ac;
    this.ds = ds;
    this.window = null;
    this.acMonitor = null;
    this.dsMonitor = null;
}
LogWindow.prototype = {
    open: function() {
        if (this.window) {
            this.window.show();
            this.window.restore();
            return;
        }
        // Create the browser window.
        this.window = new BrowserWindow({ width: 700, height: 500, icon: appIcon });
        this.window.loadURL('file://' + __dirname + '/log.html');

        if (!debug) {
            this.window.setMenu(null);
        }

        this.window.on('closed', function() {
            this.window = null;
        }.bind(this));
    },
    close: function() {
        if (this.window) {
            this.window.close();
        }
    }
};

function goHomeClicked() {
    if (interfacePath) {
        startInterface('hifi://localhost');
    } else {
        // show an error to say that we can't go home without an interface instance
        dialog.showErrorBox("Client Not Found", binaryMissingMessage("High Fidelity client", "Interface", false));
    }
}

function stackManagerBasePath() {
    var dataPath = 'High Fidelity/Stack Manager/resources';

    if (process.platform == "win32") {
        return path.resolve(osHomeDir(), 'AppData/Local', dataPath);
    } else if (process.platform == "darwin") {
        return path.resolve(osHomeDir(), 'Library/Application Support', dataPath);
    } else {
        return ""
    }
}

function isStackManagerContentPresent() {
    var modelsPath = path.resolve(stackManagerBasePath(), 'models.json.gz');

    try {
        var stats = fs.lstatSync(modelsPath);

        if (stats.isFile()) {
            console.log("Stack Manager entities file discovered at " + modelsPath)
            // we found a content file
            return true;
        }
    } catch (e) {
        console.log("Stack Manager entities file not found at " + modelsPath);
    }
}

function promptToMigrateContent() {
    dialog.showMessageBox({
        type: 'question',
        buttons: ['Yes', 'No'],
        title: 'Migrate Content',
        message: 'Are you sure?\n\nThis will stop your home server and replace everything in your home with your content from Stack Manager.'
    }, function(index) {
        if (index == 0) {
            if (homeServer.state != ProcessGroupStates.STOPPED) {
                var stopThenMigrateCallback = function(processGroup) {
                    if (isShuttingDown) {
                        homeServer.removeListener('state-update', stopThenMigrateCallback);
                    } else if (processGroup.state == ProcessGroupStates.STOPPED) {
                        performContentMigration();

                        homeServer.removeListener('state-update', stopThenMigrateCallback);
                    }
                };

                homeServer.on('state-update', stopThenMigrateCallback);
                homeServer.stop();

            } else {
                performContentMigration();
            }
        }
    });
}

function performContentMigration() {
    // check if there is a models file to migrate
    var modelsPath = path.resolve(stackManagerBasePath(), 'models.json.gz');

    try {
        var stats = fs.lstatSync(modelsPath);
    } catch (e) {
        // no entities file
        dialog.showMessageBox({
            type: 'info',
            buttons: ['OK'],
            title: 'Models File Not Found',
            message: 'There is no models file at ' + modelsPath + '\n\nStack Manager content migration can not proceed.'
        }, null);

        return;
    }

    function showMigrationCompletionDialog(copyError) {
        if (!copyError) {
            // show message for successful migration
            dialog.showMessageBox({
                type: 'info',
                buttons: ['OK'],
                title: 'Migration Complete',
                message: 'Your Stack Manager content has been migrated.\n\nYour home server will now be restarted.'
            }, null);
        } else {
            // show error message for copy fail
            dialog.showMessageBox({
                type: 'info',
                buttons: ['OK'],
                title: 'Migration Failed',
                message: 'There was an error copying your Stack Manager content: ' + copyError + '\n\nPlease try again.'
            }, null);
        }
    }

    // we have a models file, try and copy it
    var newModelsPath = path.resolve(getAssignmentClientResourcesDirectory(), 'entities/models.json.gz')
    console.log("Copying Stack Manager entity file from " + modelsPath + " to " + newModelsPath);

    try {
        fs.copySync(modelsPath, newModelsPath);

        // check if there are any assets to copy
        var oldAssetsPath = path.resolve(stackManagerBasePath(), 'assets');

        var assets = fs.readdirSync(oldAssetsPath);

        if (assets.length > 0) {
            // assume this means the directory is not empty
            // and that we should copy it
            var newAssetsPath = path.resolve(getAssignmentClientResourcesDirectory(), 'assets');

            console.log("Copying Stack Manager assets from " + oldAssetsPath + " to " + newAssetsPath);

            // attempt to copy the assets folder
            fs.copySync(oldAssetsPath, newAssetsPath, {
                preserveTimestamps: true
            });
        }

        showMigrationCompletionDialog(null);
    } catch (error) {
        showMigrationCompletionDialog(error);
    }

    homeServer.start();
}

var logWindow = null;

function buildMenuArray(serverState) {
    var menuArray = null;

    if (isShuttingDown) {
        menuArray = [
            {
                label: "Shutting down...",
                enabled: false
            }
        ];
    } else {
        menuArray = [
            {
                label: 'Server - Stopped',
                enabled: false
            },
            {
                type: 'separator'
            },
            {
                label: 'Go Home',
                click: goHomeClicked,
                enabled: false
            },
            {
                type: 'separator'
            },
            {
                label: 'Start Server',
                click: function() { homeServer.restart(); }
            },
            {
                label: 'Stop Server',
                visible: false,
                click: function() { homeServer.stop(); }
            },
            {
                label: 'Settings',
                click: function() { shell.openExternal('http://localhost:40100/settings'); },
                enabled: false
            },
            {
                label: 'View Logs',
                click: function() { logWindow.open(); }
            },
            {
                type: 'separator'
            },
            {
                label: 'Share',
                click: function() { shell.openExternal('http://localhost:40100/settings/?action=share') }
            },
            {
                type: 'separator'
            },
            {
                label: 'Quit',
                accelerator: 'Command+Q',
                click: function() { shutdown(); }
            }
        ];

        var foundStackManagerContent = isStackManagerContentPresent();
        if (foundStackManagerContent) {
            // add a separator and the stack manager content migration option
            menuArray.splice(menuArray.length - 1, 0, {
                label: 'Migrate Stack Manager Content',
                click: function() { promptToMigrateContent(); }
            }, {
                type: 'separator'
            });
        }

        updateMenuArray(menuArray, serverState);
    }

    return menuArray;
}

const GO_HOME_INDEX = 2;
const SERVER_LABEL_INDEX = 0;
const RESTART_INDEX = 4;
const STOP_INDEX = 5;
const SETTINGS_INDEX = 6;

function updateMenuArray(menuArray, serverState) {
    // update the tray menu state
    var running = serverState == ProcessGroupStates.STARTED;

    var serverLabelItem = menuArray[SERVER_LABEL_INDEX];
    var restartItem = menuArray[RESTART_INDEX];

    // Go Home is only enabled if running
    menuArray[GO_HOME_INDEX].enabled = running;

    // Stop is only visible if running
    menuArray[STOP_INDEX].visible = running;

    // Settings is only visible if running
    menuArray[SETTINGS_INDEX].enabled = running;

    if (serverState == ProcessGroupStates.STARTED) {
        serverLabelItem.label = "Server - Started";
        restartItem.label = "Restart Server";
    } else if (serverState == ProcessGroupStates.STOPPED) {
        serverLabelItem.label = "Server - Stopped";
        restartItem.label = "Start Server";
    } else if (serverState == ProcessGroupStates.STOPPING) {
        serverLabelItem.label = "Server - Stopping";

        restartItem.label = "Restart Server";
        restartItem.enabled = false;
    }
}

function updateTrayMenu(serverState) {
    if (tray) {
        var menuArray = buildMenuArray(isShuttingDown ? null : serverState);
        tray.setImage(trayIcons[serverState]);
        tray.setContextMenu(Menu.buildFromTemplate(menuArray));
        if (isShuttingDown) {
            tray.setToolTip('High Fidelity - Shutting Down');
        }
    }
}

const httpStatusPort = 60332;

function maybeInstallDefaultContentSet(onComplete) {
    // Check for existing AC data
    const acResourceDirectory = getAssignmentClientResourcesDirectory();
    console.log("Checking for existence of " + acResourceDirectory);
    var userHasExistingServerData = true;
    try {
        fs.accessSync(acResourceDirectory);
    } catch (e) {
        console.log(e);
        userHasExistingServerData = false;
    }

    if (userHasExistingServerData) {
        console.log("User has existing data, suppressing downloader");
        onComplete();
        return;
    }

    // Show popup
    var window = new BrowserWindow({
        icon: appIcon,
        width: 640,
        height: 480,
        center: true,
        frame: true,
        useContentSize: true,
        resizable: false
    });
    window.loadURL('file://' + __dirname + '/downloader.html');
    if (!debug) {
        window.setMenu(null);
    }
    window.show();

    window.on('closed', onComplete);

    electron.ipcMain.on('ready', function() {
        console.log("got ready");
        function sendStateUpdate(state, args) {
            // console.log(state, window, args);
            window.webContents.send('update', { state: state, args: args });
        }

        var aborted = false;

        // Start downloading content set
        var req = progress(request.get({
            url: "https://s3.amazonaws.com/hifi-public/homeset/ContentSet-Lounge.zip"
        }, function(error, responseMessage, responseData) {
            if (aborted) {
                return;
            } else if (error || responseMessage.statusCode != 200) {
                var message = '';
                if (error) {
                    message = "Error contacting resource server.";
                } else {
                    message = "Error downloading resources from server.";
                }
                sendStateUpdate('error', {
                    message: message
                });
            } else {
                sendStateUpdate('installing');
            }
        }), { throttle: 250 }).on('progress', function(state) {
            if (!aborted) {
                // Update progress popup
                sendStateUpdate('downloading', state);
            }
        });
        var unzipper = unzip.Extract({
            path: acResourceDirectory,
            verbose: true
        });
        unzipper.on('close', function() {
            console.log("Done", arguments);
            sendStateUpdate('complete');
        });
        unzipper.on('error', function (err) {
            console.log("aborting");
            aborted = true;
            req.abort();
            console.log("ERROR");
            sendStateUpdate('error', {
                message: "Error installing resources."
            });
        });
        req.pipe(unzipper);

        window.on('closed', function() {
            if (currentState == 'downloading') {
                req.abort();
            }
        });

        userConfig.set('hasRun', true);
    });
}

function maybeShowSplash() {
    var suppressSplash = userConfig.get('doNotShowSplash', false);

    if (!suppressSplash) {
        const zoomFactor = 0.8;
        var window = new BrowserWindow({
            icon: appIcon,
            width: 1600 * zoomFactor,
            height: 737 * zoomFactor,
            center: true,
            frame: true,
            useContentSize: true,
            zoomFactor: zoomFactor,
            resizable: false
        });
        window.loadURL('file://' + __dirname + '/splash.html');
        if (!debug) {
            window.setMenu(null);
        }
        window.show();

        window.webContents.on('new-window', function(e, url) {
            e.preventDefault();
            require('shell').openExternal(url);
        });
    }
}

const trayIconOS = (osType == "Darwin") ? "osx" : "win";
var trayIcons = {};
trayIcons[ProcessGroupStates.STARTED] = "console-tray-" + trayIconOS + ".png";
trayIcons[ProcessGroupStates.STOPPED] = "console-tray-" + trayIconOS + "-stopped.png";
trayIcons[ProcessGroupStates.STOPPING] = "console-tray-" + trayIconOS + "-stopping.png";
for (var key in trayIcons) {
    var fullPath = path.join(__dirname, '../resources/' + trayIcons[key]);
    var img = nativeImage.createFromPath(fullPath);
    img.setTemplateImage(osType == 'Darwin');
    trayIcons[key] = img;
}


const notificationIcon = path.join(__dirname, '../resources/console-notification.png');

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
app.on('ready', function() {

    if (app.dock) {
        // hide the dock icon on OS X
        app.dock.hide();
    }

    // Create tray icon
    tray = new Tray(trayIcons[ProcessGroupStates.STOPPED]);
    tray.setToolTip('High Fidelity Server Console');

    tray.on('click', function() {
        tray.popUpContextMenu(tray.menu);
    });

    updateTrayMenu(ProcessGroupStates.STOPPED);

    maybeInstallDefaultContentSet(function() {
        maybeShowSplash();

        if (buildInfo.releaseType == 'PRODUCTION') {
            var currentVersion = null;
            try {
                currentVersion = parseInt(buildInfo.buildIdentifier);
            } catch (e) {
            }

            if (currentVersion !== null) {
                const CHECK_FOR_UPDATES_INTERVAL_SECONDS = 60 * 30;
                var hasShownUpdateNotification = false;
                const updateChecker = new updater.UpdateChecker(currentVersion, CHECK_FOR_UPDATES_INTERVAL_SECONDS);
                updateChecker.on('update-available', function(latestVersion, url) {
                    if (!hasShownUpdateNotification) {
                        notifier.notify({
                            icon: notificationIcon,
                            title: 'An update is available!',
                            message: 'High Fidelity version ' + latestVersion + ' is available',
                            wait: true,
                            url: url
                        });
                        hasShownUpdateNotification = true;
                    }
                });
                notifier.on('click', function(notifierObject, options) {
                    console.log("Got click", options.url);
                    shell.openExternal(options.url);
                });
            }
        }

        deleteOldFiles(logPath, DELETE_LOG_FILES_OLDER_THAN_X_SECONDS, LOG_FILE_REGEX);

        if (dsPath && acPath) {
            domainServer = new Process('domain-server', dsPath, ["--get-temp-name"], logPath);
            acMonitor = new ACMonitorProcess('ac-monitor', acPath, ['-n6',
                                                                    '--log-directory', logPath,
                                                                    '--http-status-port', httpStatusPort], httpStatusPort, logPath);
            homeServer = new ProcessGroup('home', [domainServer, acMonitor]);
            logWindow = new LogWindow(acMonitor, domainServer);

            var processes = {
                home: homeServer
            };

            // handle process updates
            homeServer.on('state-update', function(processGroup) { updateTrayMenu(processGroup.state); });

            // start the home server
            homeServer.start();
        }
    });
});
