//
//  Created by Bradley Austin Davis on 2015/08/08
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <QObject>

#include "Forward.h"

class PluginManager : public QObject {
public:
  static PluginManager* getInstance();
  PluginManager();

  const DisplayPluginList& getDisplayPlugins();
  void disableDisplayPlugin(const QString& name);
  const InputPluginList& getInputPlugins();
  void saveSettings();
};
