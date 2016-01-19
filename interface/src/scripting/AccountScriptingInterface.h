//
//  AccountScriptingInterface.h
//  interface/src/scripting
//
//  Created by Stojce Slavkovski on 6/07/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AccountScriptingInterface_h
#define hifi_AccountScriptingInterface_h

#include <QObject>

class AccountScriptingInterface : public QObject {
    Q_OBJECT
    AccountScriptingInterface();

signals:
    void balanceChanged(float newBalance);
    
public slots:
    static AccountScriptingInterface* getInstance();
    float getBalance();
    QString getUsername();
    bool isLoggedIn();
    void updateBalance();
};

#endif // hifi_AccountScriptingInterface_h
