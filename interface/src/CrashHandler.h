//
//  CrashHandler.h
//  interface/src
//
//  Created by David Rowe on 24 Aug 2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_CrashHandler_h
#define hifi_CrashHandler_h

#include <QString>

class CrashHandler {

public:
    static void checkForAndHandleCrash();

    static void writeRunningMarkerFiler();
    static void deleteRunningMarkerFile();

private:
    enum Action {
        DELETE_INTERFACE_INI,
        RETAIN_AVATAR_INFO,
        DO_NOTHING
    };

    static Action promptUserForAction();
    static void handleCrash(Action action);

    static const QString runningMarkerFilePath();
};

#endif // hifi_CrashHandler_h
